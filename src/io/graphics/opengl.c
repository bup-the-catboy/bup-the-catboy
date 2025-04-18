#ifdef RENDERER_OPENGL

#include "io/io.h"
#include "io/sdlgfx.h"
#include "main.h"
#include "rng.h"

#ifdef WINDOWS
#define GLEW_STATIC
#endif

#include <GL/glew.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void* window;
void* gl_context;
struct GfxResource* current_texture;
float res_width, res_height;
float win_width, win_height;
float view_width, view_height;
int scissor_x, scissor_y, scissor_w, scissor_h;

#define MAX_QUADS 256
#define QUAD_SIZE 5

struct Vertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

GLuint vao, vbo, ebo;
GLuint empty_texture;
GLuint rendertexture_id, framebuffer_id;
struct Vertex vertices[4 * MAX_QUADS];
int indices[6 * MAX_QUADS];
int vertex_ptr = 0;
struct GfxResource* current_shader = 0;
struct GfxResource* dummy_shader = 0;

#define _ "\n"

const char* dummy_shader_vertex =
    "attribute vec2 a_pos;"_
    "attribute vec2 a_coord;"_
    "attribute vec4 a_color;"_
    ""_
    "varying vec2 v_coord;"_
    "varying vec4 v_color;"_
    ""_
    "void main() {"_
    "    gl_Position = vec4(a_pos, 0.0, 1.0);"_
    "    v_coord = a_coord;"_
    "    v_color = a_color;"_
    "}";

const char* dummy_shader_fragment =
    "void main() {"_
    "    gl_FragColor = texture2D(u_texture, v_coord) * v_color;"_
    "}";

const char* shader_common =
    "#version 330"_
    ""_
    "varying vec2 v_coord;"_
    "varying vec4 v_color;"_
    ""_
    "uniform sampler2D u_texture;"_
    "uniform int u_timer;"_
    "uniform int u_width;"_
    "uniform int u_height;"_
    "uniform float u_rng;"_
    ""_
    "float random(inout float seed) {"_
    "    return seed = fract(sin(dot(vec3(v_coord, seed), vec3(12.9898, 78.233, 37.719))) * 43758.5453);"_
    "}";

#undef _

void graphics_flush() {
    if (vertex_ptr == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_ptr * sizeof(struct Vertex), vertices);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vertex_ptr * 6 / 4, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    vertex_ptr = 0;
    glFlush();
}

void graphics_update_shader_params() {
    graphics_shader_set_int("u_timer",  global_timer + render_interpolation);
    graphics_shader_set_int("u_width",  res_width);
    graphics_shader_set_int("u_height", res_height);
    graphics_shader_set_float("u_rng", random_float());
}

void graphics_init_framebuffer() {
    glGenFramebuffers(1, &framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    glGenTextures(1, &rendertexture_id);
    glBindTexture(GL_TEXTURE_2D, rendertexture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_width, win_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendertexture_id, 0);
    glClearColor(.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void graphics_draw_framebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, rendertexture_id);
    glUseProgram(current_shader->shader_id);
    graphics_update_shader_params();
    float x1 = ((scissor_x + 0)         / (float)win_width)  * 2 - 1;
    float y1 = ((scissor_y + 0)         / (float)win_height) * 2 - 1;
    float x2 = ((scissor_x + scissor_w) / (float)win_width)  * 2 - 1;
    float y2 = ((scissor_y + scissor_h) / (float)win_height) * 2 - 1;
    glScissor(scissor_x, scissor_y, scissor_w, scissor_h);
    struct Vertex quad[] = {
        { x1, y1, 0, 0, 1, 1, 1, 1, },
        { x2, y1, 1, 0, 1, 1, 1, 1, },
        { x2, y2, 1, 1, 1, 1, 1, 1, },
        { x1, y2, 0, 1, 1, 1, 1, 1, },
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad), quad);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
    glUseProgram(dummy_shader->shader_id);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    struct GfxResource* tex = current_texture;
    current_texture = NULL;
    graphics_select_texture(tex);
}

void graphics_deinit_framebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer_id);
    glDeleteTextures(1, &rendertexture_id);
}

struct GfxResource* graphics_load_texture(unsigned char* buf, size_t len) {
    struct GfxResource* res = malloc(sizeof(struct GfxResource));
    unsigned char* image = stbi_load_from_memory(buf, len, &res->texture.width, &res->texture.height, NULL, STBI_rgb_alpha);
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, res->texture.width, res->texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    res->type = GfxResType_Texture;
    res->texture.texture_handle = (void*)(uintptr_t)handle;
    return res;
}

void graphics_init(const char* window_name, int width, int height) {
    sdl_window_init(window_name, width, height, SDL_GFX_API_OPENGL, &window, &gl_context);
    glewInit();
    for (int i = 0; i < MAX_QUADS; i++) {
        int vindex = i * 4;
        int iindex = i * 6;
        indices[iindex + 0] = vindex + 0;
        indices[iindex + 1] = vindex + 1;
        indices[iindex + 2] = vindex + 2;
        indices[iindex + 3] = vindex + 2;
        indices[iindex + 4] = vindex + 3;
        indices[iindex + 5] = vindex + 0;
    }
    uint32_t pixel = 0xFFFFFFFF;
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &empty_texture);
    glBindTexture(GL_TEXTURE_2D, empty_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, x));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, u));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, r));
    glBindVertexArray(0);
    current_shader = dummy_shader = graphics_load_shader(dummy_shader_fragment);
}

void graphics_set_resolution(float width, float height) {
    res_width  = width;
    res_height = height;
}

void graphics_start_frame() {
    int width, height;
    sdl_window_size(window, &width, &height);
    win_width  = width;
    win_height = height;
    view_width  = win_width;
    view_height = win_height;
    float target_aspect_ratio = res_width / res_height;
    float aspect_ratio = win_width / win_height;
    if (target_aspect_ratio > aspect_ratio) view_height = width / target_aspect_ratio;
    else view_width = height * target_aspect_ratio;
    scissor_x = round((win_width  - view_width)  / 2);
    scissor_y = round((win_height - view_height) / 2);
    scissor_w = view_width;
    scissor_h = view_height;
    view_width  /= win_width;
    view_height /= win_height;
    glViewport(0, 0, win_width, win_height);
    current_shader = dummy_shader;
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    graphics_init_framebuffer();
}

void graphics_end_frame() {
    graphics_flush();
    graphics_draw_framebuffer();
    graphics_deinit_framebuffer();
    glFlush();
    sdl_opengl_flush(window);
}

void graphics_get_size(int* width, int* height) {
    sdl_window_size(window, width, height);
}

void graphics_select_texture(struct GfxResource* texture) {
    if (current_texture == texture) return;
    current_texture = texture;
    graphics_flush();
    if (texture) glBindTexture(GL_TEXTURE_2D, (uintptr_t)texture->texture.texture_handle);
    else glBindTexture(GL_TEXTURE_2D, empty_texture);
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    x1 = (x1 / res_width)  *  2 - 1;
    x2 = (x2 / res_width)  *  2 - 1;
    y1 = (y1 / res_height) * -2 + 1;
    y2 = (y2 / res_height) * -2 + 1;
    if (vertex_ptr == MAX_QUADS * 4) graphics_flush();
    float r = ((color >> 24) & 0xFF) / 255.f;
    float g = ((color >> 16) & 0xFF) / 255.f;
    float b = ((color >>  8) & 0xFF) / 255.f;
    float a = ((color >>  0) & 0xFF) / 255.f;
    vertices[vertex_ptr++] = (struct Vertex){ x1, y1, u1, v1, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x2, y1, u2, v1, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x2, y2, u2, v2, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x1, y2, u1, v2, r, g, b, a };
}

void graphics_deinit() {
    sdl_window_deinit(window);
}

#define check_error(handle, get, attr, log, msg) { \
    GLint success;                                  \
    get(handle, attr, &success);                     \
    if (!success) {                                   \
        char infolog[512];                             \
        log(handle, 512, NULL, infolog);                \
        fprintf(stderr, "%s:\n%s\n", msg, infolog);      \
    }                                                     \
}

#define check_compile_error(handle) check_error(handle, glGetShaderiv,  GL_COMPILE_STATUS, glGetShaderInfoLog,  "Shader failed to compile")
#define check_link_error(   handle) check_error(handle, glGetProgramiv, GL_LINK_STATUS,    glGetProgramInfoLog, "Shader failed to link")

struct GfxResource* graphics_load_shader(const char* shader) {
    struct GfxResource* res = malloc(sizeof(struct GfxResource));
    res->type = GfxResType_Shader;

    char* code = malloc(strlen(shader_common) + strlen(shader) + 1);
    strcpy(code, shader_common);
    strcat(code, shader);

    int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &dummy_shader_vertex, NULL);
    glCompileShader(vertex);
    check_compile_error(vertex);

    int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&code, NULL);
    glCompileShader(fragment);
    check_compile_error(fragment);

    int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    check_link_error(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(code);

    printf("creating shader: %d\n", program);

    res->shader_id = program;
    return res;
}

void graphics_set_shader(struct GfxResource* shader) {
    graphics_flush();
    graphics_draw_framebuffer();
    current_shader = shader == NULL ? dummy_shader : shader;
}

void graphics_shader_set_int(const char* name, int value) {
    GLint last_shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_shader);
    glUseProgram(current_shader->shader_id);
    glUniform1i(glGetUniformLocation(current_shader->shader_id, name), value);
    glUseProgram(last_shader);
}

void graphics_shader_set_float(const char* name, float value) {
    GLint last_shader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_shader);
    glUseProgram(current_shader->shader_id);
    glUniform1f(glGetUniformLocation(current_shader->shader_id, name), value);
    glUseProgram(last_shader);
}

struct GfxResource* graphics_dummy_shader() {
    return dummy_shader;
}

#endif
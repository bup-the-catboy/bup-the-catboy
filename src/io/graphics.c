#include "io.h"

#include <SDL2/SDL.h>

#ifndef LEGACY_GL
#include <GL/glew.h>
#endif
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Uint64 start_ticks;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext* gl_context;
struct Texture* current_texture;
float res_width, res_height;
float win_width, win_height;

#ifndef LEGACY_GL

#define MAX_QUADS 256
#define QUAD_SIZE 5

struct Vertex {
    float x, y;
    float u, v;
    float r, g, b, a;
};

GLuint vao, vbo, ebo;
GLuint empty_texture;
struct Vertex vertices[4 * MAX_QUADS];
int indices[6 * MAX_QUADS];
int vertex_ptr = 0;
int current_shader = 0;
int dummy_shader = 0;

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
    "varying vec2 v_coord;"_
    "varying vec4 v_color;"_
    ""_
    "uniform sampler2D u_texture;"_
    ""_
    "void main() {"_
    "    gl_FragColor = texture2D(u_texture, v_coord) * v_color;"_
    "}";

#undef _

#endif

struct Texture* graphics_load_texture(unsigned char* buf, size_t len) {
    struct Texture* texture = malloc(sizeof(struct Texture));
    unsigned char* image = stbi_load_from_memory(buf, len, &texture->width, &texture->height, NULL, STBI_rgb_alpha);
    glGenTextures(1, (GLuint*)&texture->texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture->texture_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);
    return texture;
}

void graphics_init(const char* window_name, int width, int height) {
    SDL_SetHint("SDL_VIDEODRIVER", "wayland");
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    gl_context = SDL_GL_CreateContext(window);
#ifndef LEGACY_GL
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, r));
    glBindVertexArray(0);
    dummy_shader = graphics_load_shader(dummy_shader_fragment);
    graphics_select_shader(0);
#endif
}

void graphics_set_resolution(float width, float height) {
    res_width  = width;
    res_height = height;
}

void graphics_start_frame() {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    win_width  = width;
    win_height = height;
    glViewport(0, 0, width, height);
    glClearColor(.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    start_ticks = SDL_GetTicks64();
}

void graphics_end_frame(float fps) {
    graphics_flush();
    SDL_GL_SwapWindow(window);
    Uint64 end_ticks = SDL_GetTicks64();
    Uint64 frame_time = end_ticks - start_ticks;
    Sint64 wait_time = (1000 / fps) - frame_time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void graphics_get_size(int* width, int* height) {
    SDL_GetWindowSize(window, width, height);
}

void graphics_select_texture(struct Texture* texture) {
    if (current_texture == texture) return;
    current_texture = texture;
    graphics_flush();
#ifdef LEGACY_GL
    if (texture) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->texture_handle : 0);
#else
    glActiveTexture(GL_TEXTURE0);
    if (texture) glBindTexture(GL_TEXTURE_2D, texture->texture_handle);
    else glBindTexture(GL_TEXTURE_2D, empty_texture);
#endif
}

void graphics_draw(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, uint32_t color) {
    float width  = win_width;
    float height = win_height;
    float target_aspect_ratio = res_width / res_height;
    float aspect_ratio = win_width / win_height;
    if (target_aspect_ratio > aspect_ratio) height = width / target_aspect_ratio;
    else width = height * target_aspect_ratio;
    width  /= win_width;
    height /= win_height;
    x1 = ((x1 / res_width)  * width  + (1 - width)  / 2) *  2 - 1;
    x2 = ((x2 / res_width)  * width  + (1 - width)  / 2) *  2 - 1;
    y1 = ((y1 / res_height) * height + (1 - height) / 2) * -2 + 1;
    y2 = ((y2 / res_height) * height + (1 - height) / 2) * -2 + 1;
#ifdef LEGACY_GL
    glColor4ub(color >> 24, color >> 16, color >> 8, color >> 0);
    glBegin(GL_QUADS);
    glTexCoord2f(u1, v1);
    glVertex2f(x1, y1);
    glTexCoord2f(u2, v1);
    glVertex2f(x2, y1);
    glTexCoord2f(u2, v2);
    glVertex2f(x2, y2);
    glTexCoord2f(u1, v2);
    glVertex2f(x1, y2);
    glEnd();
#else
    if (vertex_ptr == MAX_QUADS * 4) graphics_flush();
    float r = ((color >> 24) & 0xFF) / 255.f;
    float g = ((color >> 16) & 0xFF) / 255.f;
    float b = ((color >>  8) & 0xFF) / 255.f;
    float a = ((color >>  0) & 0xFF) / 255.f;
    vertices[vertex_ptr++] = (struct Vertex){ x1, y1, u1, v1, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x2, y1, u2, v1, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x2, y2, u2, v2, r, g, b, a };
    vertices[vertex_ptr++] = (struct Vertex){ x1, y2, u1, v2, r, g, b, a };
#endif
}

void graphics_flush() {
#ifndef LEGACY_GL
    if (vertex_ptr == 0) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_ptr * sizeof(struct Vertex), vertices);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vertex_ptr * 6 / 4, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    vertex_ptr = 0;
#endif
    glFlush();
}

void graphics_deinit() {
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
}

#ifndef LEGACY_GL

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

int graphics_load_shader(const char* shader) {
    int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &dummy_shader_vertex, NULL);
    glCompileShader(vertex);
    check_compile_error(vertex);

    int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &shader, NULL);
    glCompileShader(fragment);
    check_compile_error(fragment);

    int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    check_link_error(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

void graphics_select_shader(int shader) {
    graphics_flush();
    glUseProgram(shader == 0 ? dummy_shader : shader);
    current_shader = shader;
}

void graphics_shader_set_int(const char* name, int value) {
    glUniform1i(glGetUniformLocation(current_shader, name), value);
}

void graphics_shader_set_float(const char* name, float value) {
    glUniform1f(glGetUniformLocation(current_shader, name), value);
}

#endif
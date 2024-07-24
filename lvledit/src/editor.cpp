#include <SDL2/SDL.h>

#include <map>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "portable-file-dialogs.h"
#include "imgui/imgui.h"
#include "writer.h"

bool inited = false;
bool init() {
    if (inited) return true;
    inited = true;
    return false;
}

float camx = 0;
float camy = 0;
int windoww = 1280;
int windowh = 720;

bool quit = false;

enum EditMode {
    EDITMODE_LAYER,
    EDITMODE_CAMBOUND
};

enum Tool {
    TOOL_BRUSH,
    TOOL_ERASER,
    TOOL_SELECTION,
    TOOL_HAND,
    TOOL_PICKER,
};

enum RenderMode {
    RENDERMODE_OPAQUE      = 255,
    RENDERMODE_TRANSLUCENT = 100,
    RENDERMODE_INVISIBLE   = 0
};

enum WindowFlag {
    WINDOWFLAG_NONE            =      0,
    WINDOWFLAG_LEVEL_SETTINGS  = 1 << 0,
    WINDOWFLAG_LAYER_SETTINGS  = 1 << 1,
    WINDOWFLAG_TILE_PALETTE    = 1 << 2,
    WINDOWFLAG_ENTITY_PALETTE  = 1 << 3,
    WINDOWFLAG_ENTITY_SETTINGS = 1 << 4,
    WINDOWFLAG_WARPS           = 1 << 5,
};

enum LayerType {
    LAYERTYPE_TILEMAP,
    LAYERTYPE_ENTITY
};

union Point {
    struct {
        int32_t x, y;
    };
    uint64_t id;
};

struct Layer {
    enum LayerType type;
    float smx, smy;
    float sox, soy;
    float scx, scy;
    struct Layer* entity_tilemap_layer;
    std::string name;
    std::map<uint64_t, int> tilemap;
};

struct ThemeData {
    const char* texture_path;
    int width, height;
    int tiles_in_row;
};

#define NO_VSCODE
#define TILESET(_1, _2) { _2 },
#define SIZE(w, h) .width = w, .height = h,
#define TILES_IN_ROW(x) .tiles_in_row = x,
#define TEXTURE(x) .texture_path = x,

struct ThemeData theme_data[] = {
#include "../../src/game/data/tilesets.h"
};

extern SDL_Renderer* renderer;
extern SDL_Window* window;

enum EditMode curr_mode = EDITMODE_LAYER;
enum Tool curr_tool = TOOL_BRUSH;
enum Tool selected_tool = TOOL_BRUSH;
enum RenderMode curr_rendermode = RENDERMODE_TRANSLUCENT;
int shown_windows = WINDOWFLAG_NONE;
bool lock_to_grid = false;

int curr_music = 0;
int curr_theme = 0;
int curr_cambound = 0;
struct Layer* curr_layer = NULL;
std::vector<std::pair<int, int>> cambounds = {};
std::vector<struct Layer> layers = {};
std::map<std::string, SDL_Texture*> texture_cache = {};
int selected_tile = 0;

int frames_drawn = 0;
int layers_created = 0;

SDL_Texture* get_texture(const char* texture_path) {
    if (texture_cache.find(texture_path) == texture_cache.end()) {
        int w, h, c;
        unsigned char* data = stbi_load((std::string("../assets/") + texture_path).c_str(), &w, &h, &c, 4);
        SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, w, h, 32, 4 * w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        texture_cache.insert({ texture_path, texture });
    }
    return texture_cache[texture_path];
}

void set_tile(int x, int y, int tile) {
    uint64_t coord = (Point){ .x = x, .y = y }.id;
    auto pos = curr_layer->tilemap.find(coord);
    if (tile == 0) {
        if (pos != curr_layer->tilemap.end()) curr_layer->tilemap.erase(pos);
    }
    else {
        if (pos == curr_layer->tilemap.end()) curr_layer->tilemap.insert({ coord, tile });
        else curr_layer->tilemap[coord] = tile;
    }
}

int get_tile(int x, int y) {
    uint64_t coord = (Point){ .x = x, .y = y }.id;
    auto pos = curr_layer->tilemap.find(coord);
    if (pos == curr_layer->tilemap.end()) return 0;
    return curr_layer->tilemap[coord];
} 

void draw_grid(SDL_Renderer* renderer, float scalex, float scaley, float speedx, float speedy, float offsetx, float offsety) {
    float grid_width  = theme_data[curr_theme].width  * 2.f;
    float grid_height = theme_data[curr_theme].height * 2.f;
    float camX = camx / scalex * speedx + offsetx;
    float camY = camy / scaley * speedy + offsety;
    int fromX = camX - 1;
    int fromY = camY - 1;
    int toX = camX + windoww / scalex / grid_width  + 1;
    int toY = camY + windowh / scaley / grid_height + 1;
    for (int y = fromY; y <= toY; y++) {
        for (int x = fromX; x <= toX; x++) {
            SDL_Rect rect = (SDL_Rect){
                .x = (int)((x - camX) * grid_width  * scalex),
                .y = (int)((y - camY) * grid_height * scaley),
                .w = (int)(grid_width  * scalex) + 1,
                .h = (int)(grid_height * scaley) + 1
            };
            SDL_RenderDrawRect(renderer, &rect);
        }
    }
}

void draw_tilemap(struct Layer* layer) {
    float grid_width  = theme_data[curr_theme].width  * 2.f;
    float grid_height = theme_data[curr_theme].height * 2.f;
    float camX = camx / layer->scx * layer->smx + layer->sox;
    float camY = camy / layer->scy * layer->smy + layer->soy;
    int fromX = camX - 1;
    int fromY = camY - 1;
    int toX = camX + windoww / layer->scx / grid_width + 1;
    int toY = camY + windowh / layer->scy / grid_height + 1;
    for (int y = fromY; y <= toY; y++) {
        for (int x = fromX; x <= toX; x++) {
            int tile = get_tile(x, y);
            SDL_Rect src = (SDL_Rect){
                .x = (tile % theme_data[curr_theme].tiles_in_row) * theme_data[curr_theme].width,
                .y = (tile / theme_data[curr_theme].tiles_in_row) * theme_data[curr_theme].height,
                .w = theme_data[curr_theme].width,
                .h = theme_data[curr_theme].height
            };
            SDL_Rect dst = (SDL_Rect){
                .x = (int)((x - camX) * grid_width  * layer->scx),
                .y = (int)((y - camY) * grid_height * layer->scy),
                .w = (int)(grid_width  * layer->scx) + 1,
                .h = (int)(grid_height * layer->scy) + 1
            };
            SDL_RenderCopy(renderer, get_texture(theme_data[curr_theme].texture_path), &src, &dst);
        }
    }
}

void add_layer(enum LayerType type) {
    struct Layer layer;
    layer.scx = layer.scy = 1;
    layer.smx = layer.smy = 1;
    layer.sox = layer.soy = 0;
    layer.type = type;
    layer.entity_tilemap_layer = NULL;
    layer.name = "Layer " + std::to_string(++layers_created);
    layers.push_back(layer);
    curr_layer = &layers[layers.size() - 1];
}

void delete_layer(int index) {
    struct Layer* layer = &layers[index];
    for (struct Layer l : layers) {
        if (l.entity_tilemap_layer == layer) l.entity_tilemap_layer = NULL;
    }
    std::string id = "";
    if (curr_layer) id = curr_layer->name;
    layers.erase(layers.begin() + index);
    if (id != "") {
        curr_layer = layers.size() == 0 ? NULL : &layers[index == layers.size() ? index - 1 : index];
        for (int i = 0; i < layers.size(); i++) {
            if (layers[i].name == id) curr_layer = &layers[i];
        }
    }
}

void move_layer(int from, int to) {
    std::string id = "";
    if (curr_layer) id = curr_layer->name;
    struct Layer temp = layers[from];
    layers[from] = layers[to];
    layers[to] = temp;
    if (id != "") {
        for (int i = 0; i < layers.size(); i++) {
            if (layers[i].name == id) curr_layer = &layers[i];
        }
    }
}

#define _
#define CTRL  c = ctrl;
#define SHIFT s = shift;
#define ALT   a = alt;
#define MODIF(modif) ({ \
    bool c = !ctrl;      \
    bool s = !shift;      \
    bool a = !alt;         \
    modif                   \
    c && s && a;             \
})
#define TOGGLE_WINDOW(flag) (shown_windows = (shown_windows & (flag) ? (shown_windows & ~(flag)) : (shown_windows | (flag))))

void get_tile_position_from_pixel(int inx, int iny, int* outx, int* outy) {
    if (!curr_layer) return;
    float grid_width  = theme_data[curr_theme].width  * 2.f;
    float grid_height = theme_data[curr_theme].height * 2.f;
    if (outx) *outx = floor(camx + (inx / grid_width ) / curr_layer->scx * curr_layer->smx + curr_layer->sox);
    if (outy) *outy = floor(camy + (iny / grid_height) / curr_layer->scy * curr_layer->smy + curr_layer->soy);
}

void stub_start(int x, int y) {}
void stub_drag(int x, int y, int dx, int dy) {}
void stub_end() {}

void tile_modif_drag(int x, int y, int tile) {
    if (!curr_layer) return;
    if (curr_layer->type != LAYERTYPE_TILEMAP) return;
    int tileX, tileY;
    get_tile_position_from_pixel(x, y, &tileX, &tileY);
    set_tile(tileX, tileY, tile);
}

void brush_drag(int x, int y, int dx, int dy) {
    tile_modif_drag(x, y, selected_tile);
}

void eraser_drag(int x, int y, int dx, int dy) {
    tile_modif_drag(x, y, 0);
}

struct {
    void(*start)(int x, int y);
    void(*drag)(int x, int y, int dx, int dy);
    void(*end)();
    char letter;
} tools[] = {
    { stub_start, brush_drag, stub_end, 'B' },
    { stub_start, eraser_drag, stub_end, 'E' },
    { stub_start, stub_drag, stub_end, 'S' },
    { stub_start, stub_drag, stub_end, 'H' },
    { stub_start, stub_drag, stub_end, 'P' },
};

void get_tilemap_size(struct Layer* layer, int* tilemap_width, int* tilemap_height, int* offset_x, int* offset_y) {
    int w = 0, h = 0, ox = 0, oy = 0;
    if (!layer->tilemap.empty()) {
        int left  = INT32_MAX;
        int right = INT32_MIN;
        int up    = INT32_MAX;
        int down  = INT32_MIN;
        for (auto tile : layer->tilemap) {
            union Point p = *(union Point*)&tile.first;
            int x = floor(p.x / 24.f);
            int y = floor(p.y / 16.f);
            if (left  > x) left  = x;
            if (right < x) right = x;
            if (up    > y) up    = y;
            if (down  < y) down  = y;
        }
        w = right - left + 1;
        h = down - up + 1;
        ox = left * -24;
        oy = up   * -16;
    }
    if (tilemap_width)  *tilemap_width  = w;
    if (tilemap_height) *tilemap_height = h;
    if (offset_x)       *offset_x       = ox;
    if (offset_y)       *offset_y       = oy;
}

char* create_tilemap_data(struct Layer* layer, int* width, int* height) {
    int screen_w, screen_h, offset_x, offset_y;
    get_tilemap_size(layer, &screen_w, &screen_h, &offset_x, &offset_y);
    int w = screen_w * 24;
    int h = screen_h * 16;
    char* data = (char*)malloc(w * h);
    memset(data, 0, w * h);
    for (auto tile : layer->tilemap) {
        union Point p = *(union Point*)&tile.first;
        data[(p.y + offset_y) * w + (p.x + offset_x)] = tile.second;
    }
    if (*width)  *width  = w;
    if (*height) *height = h;
    return data;
}

std::string last_saved = "";

void save_file(bool force_select) {
    if (last_saved == "" || force_select) {
        last_saved = pfd::save_file("Save Level", "", { "Level Files", "*.lvl", "All Files", "*" }).result();
    }

    WriteStream* stream = writer_create(16);
    writer_make_offset(stream, 12);
    writer_write<int32_t>(stream, curr_theme);
    writer_write<int32_t>(stream, curr_music);
    writer_write<int32_t>(stream, curr_cambound);
    writer_pop_block(stream);

    writer_make_offset(stream, 4);
    writer_write<int32_t>(stream, 0);
    writer_pop_block(stream);

    writer_make_offset(stream, 4);
    writer_write<int32_t>(stream, 0);
    writer_pop_block(stream);

    writer_make_offset(stream, 4 + layers.size() * 4);
    writer_write<int32_t>(stream, layers.size());
    for (struct Layer layer : layers) {
        writer_make_offset(stream, 32);
        writer_write<int32_t>(stream, layer.type);
        writer_write_ptr(stream, &layer.smx, sizeof(float));
        writer_write_ptr(stream, &layer.smy, sizeof(float));
        writer_write_ptr(stream, &layer.sox, sizeof(float));
        writer_write_ptr(stream, &layer.soy, sizeof(float));
        writer_write_ptr(stream, &layer.scx, sizeof(float));
        writer_write_ptr(stream, &layer.scy, sizeof(float));

        int width, height;
        char* tilemap_data;
        switch (layer.type) {
            case LAYERTYPE_TILEMAP:
            tilemap_data = create_tilemap_data(&layer, &width, &height);
            writer_make_offset(stream, 8 + width * height);
            writer_write<int32_t>(stream, width / 24);
            writer_write<int32_t>(stream, height / 16);
            for (int i = 0; i < width * height; i++) {
                writer_write_ptr(stream, tilemap_data + i, 1);
            }
            writer_pop_block(stream);
            free(tilemap_data);
            break;

            case LAYERTYPE_ENTITY:
            writer_make_null_offset(stream);
            break;
        }

        writer_pop_block(stream);
    }

    int size;
    char* data = writer_close(stream, &size);
    FILE* f = fopen(last_saved.c_str(), "w");
    fwrite(data, size, 1, f);
    fclose(f);
    free(data);
}

void activate_shortcut(bool ctrl, bool shift, bool alt, char letter) {
    if (MODIF(CTRL)) {
        switch (letter) {
            case 'N': break; // todo
            case 'O': break; // todo
            case 'S': save_file(false); break;
            case 'Q': quit = true; break;
            case 'L': curr_mode = EDITMODE_LAYER; break;
            case 'B': curr_mode = EDITMODE_CAMBOUND; break;
            case 'Z': break; // todo
            case 'C': break; // todo
            case 'V': break; // todo
            case 'X': break; // todo
            case 'T': add_layer(LAYERTYPE_TILEMAP); break; // todo
            case 'E': add_layer(LAYERTYPE_ENTITY); break; // todo
        }
    }
    if (MODIF(CTRL SHIFT)) {
        switch (letter) {
            case 'S': save_file(true); break; // todo
            case 'Z': break; // todo
            case 'O': curr_rendermode = RENDERMODE_OPAQUE; break;
            case 'T': curr_rendermode = RENDERMODE_TRANSLUCENT; break;
            case 'I': curr_rendermode = RENDERMODE_INVISIBLE; break;
            case 'L': lock_to_grid ^= 1; break;
        }
    }
    if (MODIF(CTRL ALT)) {
        switch (letter) {
            case 'S': TOGGLE_WINDOW(WINDOWFLAG_LEVEL_SETTINGS); break;
            case 'L': TOGGLE_WINDOW(WINDOWFLAG_LAYER_SETTINGS); break;
            case 'T': TOGGLE_WINDOW(WINDOWFLAG_TILE_PALETTE); break;
            case 'E': TOGGLE_WINDOW(WINDOWFLAG_ENTITY_PALETTE); break;
            case 'P': TOGGLE_WINDOW(WINDOWFLAG_ENTITY_SETTINGS); break;
            case 'W': TOGGLE_WINDOW(WINDOWFLAG_WARPS); break;
        }
    }
    if (MODIF(_)) {
        switch (letter) {
            case 'B': selected_tool = TOOL_BRUSH; break;
            case 'E': selected_tool = TOOL_ERASER; break;
            case 'S': selected_tool = TOOL_SELECTION; break;
            case 'H': selected_tool = TOOL_HAND; break;
            case 'P': selected_tool = TOOL_PICKER; break;
        }
    }
}

bool should_quit() {
    return quit;
}

#undef CTRL
#undef SHIFT
#undef ALT
#undef _
#define _              false, false, false,
#define CTRL           true,  false, false,
#define SHIFT          false, true,  false,
#define ALT            false, false, true,
#define CTRL_SHIFT     true,  true,  false,
#define CTRL_ALT       true,  false, true,
#define CTRL_SHIFT_ALT true,  true,  true,
#define SHIFT_ALT      false, true,  true,

#define WINDOW(flag, title) if (({       \
    bool run = shown_windows & (flag);    \
    if (run) {                             \
        bool shown = true;                  \
        ImGui::Begin(title, &shown);         \
        if (!shown) shown_windows &= ~(flag); \
    }                                          \
    run;                                        \
}))

void editor_run(SDL_Renderer* renderer) {
    SDL_GetWindowSize(window, &windoww, &windowh);

    if (ImGui::BeginMainMenuBar()) {
        ImGui::BeginDisabled();
        ImGui::Text("(%c)", tools[selected_tool].letter);
        ImGui::EndDisabled();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) activate_shortcut(CTRL 'N');
            if (ImGui::MenuItem("Open", "Ctrl+O")) activate_shortcut(CTRL 'O');
            if (ImGui::MenuItem("Save", "Ctrl+S")) activate_shortcut(CTRL 'S');
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) activate_shortcut(CTRL_SHIFT 'S');
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) activate_shortcut(CTRL 'Q');
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::SeparatorText("Modes");
            if (ImGui::MenuItem("Layer Mode", "Ctrl+L", curr_mode == EDITMODE_LAYER)) activate_shortcut(CTRL 'L');
            if (ImGui::MenuItem("Camera Bound Mode", "Ctrl+B", curr_mode == EDITMODE_CAMBOUND)) activate_shortcut(CTRL 'B');
            ImGui::SeparatorText("Actions");
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) activate_shortcut(CTRL 'Z');
            if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) activate_shortcut(CTRL_SHIFT 'Z');
            if (ImGui::MenuItem("Copy", "Ctrl+C")) activate_shortcut(CTRL 'C');
            if (ImGui::MenuItem("Paste", "Ctrl+V")) activate_shortcut(CTRL 'V');
            if (ImGui::MenuItem("Cut", "Ctrl+X")) activate_shortcut(CTRL 'X');
            ImGui::SeparatorText("Tools");
            if (ImGui::MenuItem("Brush", "B", selected_tool == TOOL_BRUSH)) activate_shortcut(_ 'B');
            if (ImGui::MenuItem("Eraser", "E", selected_tool == TOOL_ERASER)) activate_shortcut(_ 'E');
            if (ImGui::MenuItem("Selection", "S", selected_tool == TOOL_SELECTION)) activate_shortcut(_ 'S');
            if (ImGui::MenuItem("Hand", "H", selected_tool == TOOL_HAND)) activate_shortcut(_ 'H');
            if (ImGui::MenuItem("Picker", "P", selected_tool == TOOL_PICKER)) activate_shortcut(_ 'P');
            ImGui::SeparatorText("Settings");
            if (ImGui::MenuItem("Lock Entities to Grid", "Ctrl+Shift+L", lock_to_grid)) activate_shortcut(CTRL_SHIFT 'L');
            ImGui::SeparatorText("Layers");
            if (ImGui::BeginMenu("Add Layer")) {
                if (ImGui::MenuItem("Tile Layer", "Ctrl+T")) activate_shortcut(CTRL 'T');
                if (ImGui::MenuItem("Entity Layer", "Ctrl+E")) activate_shortcut(CTRL 'E');
                ImGui::EndMenu();
            }
            for (int i = 0; i < layers.size(); i++) {
                if (ImGui::SmallButton(("X##" + std::to_string(i)).c_str())) delete_layer(i); ImGui::SameLine();
                ImGui::BeginDisabled(i == 0);
                if (ImGui::SmallButton(("^##" + std::to_string(i)).c_str())) move_layer(i, i - 1); ImGui::SameLine();
                ImGui::EndDisabled();
                ImGui::BeginDisabled(i == layers.size() - 1);
                if (ImGui::SmallButton(("v##" + std::to_string(i)).c_str())) move_layer(i, i + 1); ImGui::SameLine();
                ImGui::EndDisabled();
                if (ImGui::MenuItem(layers[i].name.c_str(), NULL, curr_layer == &layers[i])) curr_layer = &layers[i];
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Render Other Layers As")) {
                if (ImGui::MenuItem("Opaque", "Ctrl+Shift+O", curr_rendermode == RENDERMODE_OPAQUE)) activate_shortcut(CTRL_SHIFT 'O');
                if (ImGui::MenuItem("Translucent", "Ctrl+Shift+T", curr_rendermode == RENDERMODE_TRANSLUCENT)) activate_shortcut(CTRL_SHIFT 'T');
                if (ImGui::MenuItem("Invisible", "Ctrl+Shift+I", curr_rendermode == RENDERMODE_INVISIBLE)) activate_shortcut(CTRL_SHIFT 'I');
                ImGui::EndMenu();
            }
            ImGui::SeparatorText("Windows");
            if (ImGui::MenuItem("Level Settings", "Ctrl+Alt+S", shown_windows & WINDOWFLAG_LEVEL_SETTINGS)) activate_shortcut(CTRL_ALT 'S');
            if (ImGui::MenuItem("Layer Settings", "Ctrl+Alt+L", shown_windows & WINDOWFLAG_LAYER_SETTINGS)) activate_shortcut(CTRL_ALT 'L');
            if (ImGui::MenuItem("Tile Palette", "Ctrl+Alt+T", shown_windows & WINDOWFLAG_TILE_PALETTE)) activate_shortcut(CTRL_ALT 'T');
            if (ImGui::MenuItem("Entity Palette", "Ctrl+Alt+E", shown_windows & WINDOWFLAG_ENTITY_PALETTE)) activate_shortcut(CTRL_ALT 'E');
            if (ImGui::MenuItem("Entity Settings", "Ctrl+Alt+P", shown_windows & WINDOWFLAG_ENTITY_SETTINGS)) activate_shortcut(CTRL_ALT 'P');
            if (ImGui::MenuItem("Warps", "Ctrl+Alt+W", shown_windows & WINDOWFLAG_WARPS)) activate_shortcut(CTRL_ALT 'W');
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

#undef TILESET
#define MUSIC(  _1, _2) #_1 "\0"
#define TILESET(_1, _2) #_1 "\0"

    const char* str_music =
#include "../../src/game/data/music.h"
    ;
    const char* str_theme =
#include "../../src/game/data/tilesets.h"
    ;

    WINDOW(WINDOWFLAG_LEVEL_SETTINGS, "Level Settings") {
        ImGui::PushItemWidth(200);
        ImGui::Combo("Music", &curr_music, str_music);
        ImGui::Combo("Theme", &curr_theme, str_theme);
        ImGui::BeginDisabled(cambounds.size() == 0);
        if (ImGui::InputInt("Camera Boundary", &curr_cambound)) {
            if (curr_cambound < 0) curr_cambound = 0;
            if (curr_cambound >= cambounds.size()) curr_cambound = cambounds.size() - 1;
        }
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::End();
    }

    WINDOW(WINDOWFLAG_LAYER_SETTINGS, "Layer Settings") {
        if (curr_layer) {
            ImGui::PushItemWidth(100);
            ImGui::SliderFloat("##scale", &curr_layer->scx, 0.1f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SameLine();
            ImGui::SliderFloat("Scale", &curr_layer->scy, 0.1f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("##scrlmult", &curr_layer->smx, 0.1f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SameLine();
            ImGui::SliderFloat("Scroll Speed", &curr_layer->smy, 0.1f, 10.f, "%.3f", ImGuiSliderFlags_Logarithmic);
            ImGui::SliderFloat("##scrloff", &curr_layer->sox, -10.f, 10.f, "%.3f");
            ImGui::SameLine();
            ImGui::SliderFloat("Scroll Offset", &curr_layer->soy, -10.f, 10.f, "%.3f");
            ImGui::PopItemWidth();
            if (curr_layer->type == LAYERTYPE_ENTITY) {
                if (ImGui::BeginCombo("Tilemap Layer", (
                    curr_layer->entity_tilemap_layer == NULL
                        ? std::string("(None)")
                        : curr_layer->entity_tilemap_layer->name
                ).c_str())) {
                    if (ImGui::Selectable("(None)")) curr_layer->entity_tilemap_layer = NULL;
                    for (int i = 0; i < layers.size(); i++) {
                        if (layers[i].type != LAYERTYPE_TILEMAP) continue;
                        if (ImGui::Selectable(layers[i].name.c_str())) curr_layer->entity_tilemap_layer = &layers[i];
                    }
                    ImGui::EndCombo();
                }
            }
        }
        else ImGui::Text("No layer is selected.");
        ImGui::End();
    }

    WINDOW(WINDOWFLAG_TILE_PALETTE, "Tile Palette") {
        int i = 0;
        const char* name = "";
        bool hide = false;
#undef TEXTURE
#define TILE(_1, _2) if (i % 5 != 0) ImGui::SameLine(); name = #_1; hide = false; _2 i++;
#define TEXTURE(_1)
#define COLLISION(_1)
#define SOLID()
#define SIMPLE_STATIONARY_TEXTURE(_1) SIMPLE_ANIMATED_TEXTURE(1, _1)
#define LVLEDIT_HIDE hide = true;
#define SIMPLE_ANIMATED_TEXTURE(_1, ...) if (!hide) {                                                                   \
            int frames[] = { __VA_ARGS__ };                                                                              \
            int curr_frame = frames[frames_drawn % (sizeof(frames) / sizeof(int))];                                       \
            int width, height;                                                                                             \
            SDL_Texture* tex = get_texture(theme_data[curr_theme].texture_path);                                            \
            SDL_QueryTexture(tex, NULL, NULL, &width, &height);                                                              \
            float u1 = (curr_frame % theme_data[curr_theme].tiles_in_row) / ((float)width  / theme_data[curr_theme].width);   \
            float v1 = (curr_frame / theme_data[curr_theme].tiles_in_row) / ((float)height / theme_data[curr_theme].height);   \
            float u2 = (curr_frame % theme_data[curr_theme].tiles_in_row + 1) / ((float)width  / theme_data[curr_theme].width); \
            float v2 = (curr_frame / theme_data[curr_theme].tiles_in_row + 1) / ((float)height / theme_data[curr_theme].height); \
            ImGui::BeginDisabled(selected_tile == i);                                                                             \
            if (ImGui::ImageButton(                                                                                                \
                ("tile" + std::to_string(i)).c_str(),                                                                               \
                tex,                                                                                                                 \
                ImVec2(                                                                                                               \
                    theme_data[curr_theme].width * 2,                                                                                  \
                    theme_data[curr_theme].height * 2                                                                                   \
                ), ImVec2(u1, v1), ImVec2(u2, v2)                                                                                        \
            )) {                                                                                                                          \
                selected_tile = i;                                                                                                         \
            }                                                                                                                               \
            ImGui::SetItemTooltip("%s", name);                                                                                               \
            ImGui::EndDisabled();                                                                                                             \
        }
#include "../../src/game/data/tiles.h"
        ImGui::End();
    }
    WINDOW(WINDOWFLAG_ENTITY_PALETTE, "Entity Palette") { ImGui::End(); }
    WINDOW(WINDOWFLAG_ENTITY_SETTINGS, "Entity Settings") { ImGui::End(); }
    WINDOW(WINDOWFLAG_WARPS, "Warps") { ImGui::End(); }

    for (int i = layers.size() - 1; i >= 0; i--) {
        int opacity;
        if (curr_layer == &layers[i]) opacity = 255;
        else opacity = curr_rendermode;
        if (opacity == 0) continue;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, opacity);
        if (layers[i].type == LAYERTYPE_TILEMAP) draw_tilemap(&layers[i]);
    }
    
    if (curr_layer) {
        struct Layer layer = *curr_layer;
        if (layer.type == LAYERTYPE_ENTITY && layer.entity_tilemap_layer) layer = *layer.entity_tilemap_layer;
        if (layer.type != LAYERTYPE_ENTITY) {
            struct ThemeData theme = theme_data[curr_theme];
            SDL_SetRenderDrawColor(renderer, 176, 176, 176, 176);
            draw_grid(renderer, (theme.width * 2) / 32.f * layer.scx, (theme.height * 2) / 32.f * layer.scy, layer.smx, layer.smy, layer.sox, layer.soy);
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    draw_grid(renderer, 24, 16, 1, 1, 0, 0);

    frames_drawn++;
}

void editor_process_event(SDL_Event* event) {
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.sym >= SDLK_a && event->key.keysym.sym <= SDLK_z) activate_shortcut(
                event->key.keysym.mod & KMOD_CTRL,
                event->key.keysym.mod & KMOD_SHIFT,
                event->key.keysym.mod & KMOD_ALT,
                event->key.keysym.sym - SDLK_a + 'A'
            );
        }
    }

    if (!ImGui::GetIO().WantCaptureMouse) {
        if (event->type == SDL_MOUSEMOTION) {
            if (event->motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                tools[curr_tool].drag(
                    event->motion.x, event->motion.y,
                    event->motion.xrel, event->motion.yrel
                );
            }
            if (event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                camx -= event->motion.xrel / 32.f;
                camy -= event->motion.yrel / 32.f;
            }
        }
        if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button == SDL_BUTTON_LEFT) {
                curr_tool = selected_tool;
                tools[curr_tool].start(event->button.x, event->button.y);
            }
        }
        if (event->type == SDL_MOUSEBUTTONUP) {
            if (event->button.button == SDL_BUTTON_LEFT) tools[curr_tool].end();
        }
    }
}
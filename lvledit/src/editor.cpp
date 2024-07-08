#include <SDL2/SDL.h>

#include "imgui/imgui.h"

#include <SDL2/SDL_render.h>
#include <vector>
#include <string>

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
    TOOL_SELECTION,
    TOOL_HAND,
    TOOL_EYEDROPPER
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

struct Layer {
    enum LayerType type;
    float smx, smy;
    float sox, soy;
    float scx, scy;
    struct Layer* entity_tilemap_layer;
    std::string name;
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

enum EditMode curr_mode = EDITMODE_LAYER;
enum Tool curr_tool = TOOL_BRUSH;
enum RenderMode curr_rendermode = RENDERMODE_TRANSLUCENT;
int shown_windows = WINDOWFLAG_NONE;
bool lock_to_grid = false;

int curr_music = 0;
int curr_theme = 0;
int curr_cambound = 0;
struct Layer* curr_layer = NULL;
std::vector<std::pair<int, int>> cambounds = {};
std::vector<struct Layer> layers = {};

int layers_created = 0;

void draw_grid(SDL_Renderer* renderer, float scalex, float scaley, float speedx, float speedy, float offsetx, float offsety) {
    float camX = camx / scalex * speedx + offsetx;
    float camY = camy / scaley * speedy + offsety;
    int fromX = camX - 1;
    int fromY = camY - 1;
    int toX = camX + windoww / scalex / 32.f + 1;
    int toY = camY + windowh / scaley / 32.f + 1;
    for (int y = fromY; y <= toY; y++) {
        for (int x = fromX; x <= toX; x++) {
            SDL_Rect rect = (SDL_Rect){ .x = (int)((x - camX) * 32 * scalex), .y = (int)((y - camY) * 32 * scaley), .w = (int)(32 * scalex) + 1, .h = (int)(32 * scaley) + 1 };
            SDL_RenderDrawRect(renderer, &rect);
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

void activate_shortcut(bool ctrl, bool shift, bool alt, char letter) {
    if (MODIF(CTRL)) {
        switch (letter) {
            case 'N': break; // todo
            case 'O': break; // todo
            case 'S': break; // todo
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
            case 'S': break; // todo
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
            case 'B': curr_tool = TOOL_BRUSH; break;
            case 'S': curr_tool = TOOL_SELECTION; break;
            case 'H': curr_tool = TOOL_HAND; break;
            case 'E': curr_tool = TOOL_EYEDROPPER; break;
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
    if (ImGui::BeginMainMenuBar()) {
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
            if (ImGui::MenuItem("Brush", "B", curr_tool == TOOL_BRUSH)) activate_shortcut(_ 'B');
            if (ImGui::MenuItem("Selection", "S", curr_tool == TOOL_SELECTION)) activate_shortcut(_ 'S');
            if (ImGui::MenuItem("Hand", "H", curr_tool == TOOL_HAND)) activate_shortcut(_ 'L');
            if (ImGui::MenuItem("Eyedropper", "E", curr_tool == TOOL_EYEDROPPER)) activate_shortcut(_ 'E');
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
        ImGui::End();
    }
    WINDOW(WINDOWFLAG_ENTITY_PALETTE, "Entity Palette") { ImGui::End(); }
    WINDOW(WINDOWFLAG_ENTITY_SETTINGS, "Entity Settings") { ImGui::End(); }
    WINDOW(WINDOWFLAG_WARPS, "Warps") { ImGui::End(); }
    
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
}

void editor_process_event(SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
        SDL_GetWindowSize(window, &windoww, &windowh);
    }

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
            if (event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                camx -= event->motion.xrel / 32.f;
                camy -= event->motion.yrel / 32.f;
            }
        }
    }
}
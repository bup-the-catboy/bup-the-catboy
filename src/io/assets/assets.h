#ifndef BTCB_ASSETS_H
#define BTCB_ASSETS_H

#define GET_ASSET(type, name) ((typeof(type)*)get_asset(name))

enum GfxResType {
    GfxResType_Texture,
    GfxResType_Shader,
};

struct Binary {
    unsigned char* ptr;
    unsigned int length;
};

struct Texture {
    void* texture_handle;
    int width;
    int height;
};

struct GfxResource {
    enum GfxResType type;
    union {
        struct Texture texture;
        int shader_id;
    };
};
;
void load_assets();
void* get_asset(const char* name);
const char* get_asset_name(void* asset);
void extract_assets();

#endif
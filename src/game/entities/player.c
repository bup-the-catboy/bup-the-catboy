#include "functions.h"

#include <SDL2/SDL.h>

#include "assets/assets.h"
#include "game/input.h"
#include "game/camera.h"
#include "game/network/common.h"
#include "game/network/packet.h"

entity_texture(player) {
    SDL_Texture* tex = GET_ASSET(SDL_Texture, "images/entities/player.png");
    SDL_QueryTexture(tex, NULL, NULL, w, h);
    *srcX = 0;
    *srcY = 0;
    *srcW = *w;
    *srcH = *h;
    return tex;
}

entity_update(player) {
    bool l = is_button_down(BUTTON_MOVE_LEFT);
    bool r = is_button_down(BUTTON_MOVE_RIGHT);
    if (l && !r) {
        entity->velX -= 0.02f;
        if (entity->velX < -0.2f) entity->velX = -0.2f;
    }
    else if (!l && r) {
        entity->velX += 0.02f;
        if (entity->velX > 0.2f) entity->velX = 0.2f;
    }
    else {
        if (entity->velX < 0) {
            entity->velX += 0.02f;
            if (entity->velX > 0) entity->velX = 0;
        }
        if (entity->velX > 0) {
            entity->velX -= 0.02f;
            if (entity->velX < 0) entity->velX = 0;
        }
    }
    entity->velY += 0.03f;
    if ((entity->flags & LE_EntityFlags_OnGround) && is_button_pressed(BUTTON_JUMP)) entity->velY = -0.5f;
    send_packet(packet_entity(entity));
}

entity_update(network_player) {

}

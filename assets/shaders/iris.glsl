#version 330

varying vec2 v_coord;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform int u_timer;
uniform int u_width;
uniform int u_height;
uniform float u_rng;

uniform float u_xpos;
uniform float u_ypos;
uniform float u_radius;

uniform float u_bgcol_r;
uniform float u_bgcol_g;
uniform float u_bgcol_b;
uniform float u_bgcol_a;

void main() {
    vec2 pos = vec2(v_coord.x * u_width, v_coord.y * u_height);
    pos.y = 256 - pos.y;
    float dist = sqrt((u_xpos - pos.x) * (u_xpos - pos.x) + (u_ypos - pos.y) * (u_ypos - pos.y));
    gl_FragColor = dist >= u_radius ? vec4(0, 0, 0, 1) : texture2D(u_texture, v_coord) * v_color * vec4(u_bgcol_r, u_bgcol_g, u_bgcol_b, u_bgcol_a);
}

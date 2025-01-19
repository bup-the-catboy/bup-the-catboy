#version 330

varying vec2 v_coord;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform int u_timer;
uniform int u_width;
uniform int u_height;

uniform float u_xpos;
uniform float u_ypos;
uniform float u_radius;

void main() {
    vec2 pos = vec2(v_coord.x * u_width, v_coord.y * u_height);
    float dist = sqrt((u_xpos - pos.x) * (u_xpos - pos.x) + (u_ypos - pos.y) * (u_ypos - pos.y));
    gl_FragColor = vec4(0, 0, 0, dist >= u_radius);
}

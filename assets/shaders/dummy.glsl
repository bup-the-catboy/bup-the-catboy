#version 330

varying vec2 v_coord;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform int u_timer;
uniform int u_width;
uniform int u_height;

void main() {
    gl_FragColor = texture2D(u_texture, v_coord) * v_color;
}

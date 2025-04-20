uniform float u_xpos;
uniform float u_ypos;
uniform float u_radius;

void main() {
    vec2 pos = vec2(v_coord.x * u_width, v_coord.y * u_height);
    pos.y = 256 - pos.y;
    float dist = sqrt((u_xpos - pos.x) * (u_xpos - pos.x) + (u_ypos - pos.y) * (u_ypos - pos.y));
    gl_FragColor = dist >= u_radius ? texture2D(u_texture, v_coord).a == 0 ? vec4(0, 0, 0, 1) : vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0);
}

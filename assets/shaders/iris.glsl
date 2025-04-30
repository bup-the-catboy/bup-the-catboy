uniform float u_xpos;
uniform float u_ypos;
uniform float u_radius;

vec4 iris(vec4 color) {
    vec2 pos = vec2(v_coord.x * u_width, v_coord.y * u_height);
    pos.y = 256 - pos.y;
    float dist = sqrt((u_xpos - pos.x) * (u_xpos - pos.x) + (u_ypos - pos.y) * (u_ypos - pos.y));
    return dist >= u_radius ? color.a == 0 ? vec4(0, 0, 0, 1) : vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0);
}

uniform float u_rot_posx;
uniform float u_rot_posy;
uniform float u_rot_angle;

vec4 rotation(vec4 color) {
    vec2 origin  = vec2(u_rot_posx, u_rot_posy);
    vec2 coord   = vec2(v_coord.x * u_width, v_coord.y * u_height);
    origin.y = u_height - origin.y;
    vec2 rotated = vec2(
        u_rot_posx + (coord.x - origin.x) * cos(u_rot_angle) - (coord.y - origin.y) * sin(u_rot_angle),
        u_rot_posy + (coord.x - origin.x) * sin(u_rot_angle) + (coord.y - origin.y) * cos(u_rot_angle)
    );
    rotated.y = u_height - rotated.y;
    return get_texture(u_texture, vec2(rotated.x / u_width, rotated.y / u_height)) * v_color;
}

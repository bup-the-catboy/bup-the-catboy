uniform float u_posx;
uniform float u_posy;
uniform float u_angle;

void main() {
    vec2 coord  = vec2(v_coord.x * u_width, v_coord.y * u_height);
    vec2 origin = coord - vec2(u_posx, u_posy);
    coord.x = cos(u_angle) * origin.x - sin(u_angle) * origin.y + u_posx;
    coord.y = sin(u_angle) * origin.x + cos(u_angle) * origin.y + u_posy;
    gl_FragColor = texture2D(u_texture, vec2(coord.x / u_width, coord.y / u_height)) * v_color;
}

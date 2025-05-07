uniform float u_iris_posx;
uniform float u_iris_posy;
uniform float u_iris_radius;

vec4 iris(vec4 color) {
    vec2 pos = vec2(v_coord.x * u_width, v_coord.y * u_height);
    pos.y = 256 - pos.y;
    float dist = sqrt((u_iris_posx - pos.x) * (u_iris_posx - pos.x) + (u_iris_posy - pos.y) * (u_iris_posy - pos.y));
    return dist >= u_iris_radius ? color.a == 0 ? vec4(0, 0, 0, 1) : vec4(1, 1, 1, 1) : vec4(0, 0, 0, 0);
}

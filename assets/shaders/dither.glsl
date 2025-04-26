uniform int u_dither_amount;
uniform float u_offset_x;
uniform float u_offset_y;

const int thresholds[] = int[16](
    1, 9, 3, 11,
    13,5, 15,7,
    4, 12,2, 10,
    16,8, 14,6
);

void main() {
    int x = int(v_coord.x * u_width  + u_offset_x) % 4;
    int y = int(v_coord.y * u_height + u_offset_y) % 4;
    int threshold = thresholds[y * 4 + x];
    if (u_dither_amount < threshold) gl_FragColor = texture2D(u_texture, v_coord) * v_color;
    else discard;
}

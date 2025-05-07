uniform int u_dither_amount;
uniform float u_dither_offx;
uniform float u_dither_offy;

const int thresholds[] = int[16](
    1, 9, 3, 11,
    13,5, 15,7,
    4, 12,2, 10,
    16,8, 14,6
);

vec4 dither(vec4 color) {
    int x = int(v_coord.x * u_width  + u_dither_offx) % 4;
    int y = int(v_coord.y * u_height + u_dither_offy) % 4;
    int threshold = thresholds[y * 4 + x];
    if (u_dither_amount < threshold) return color;
    else discard;
}

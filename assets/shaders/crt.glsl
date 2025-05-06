uniform float u_flicker_strength = 0.05;
uniform float u_distortion_amount = 0.5;
uniform float u_chromatic_aberration = 0.015;
uniform float u_scanline_brightness = 0.8;
uniform float u_scanline_amount = 480;
uniform float u_fuzziness = .5f;

vec4 get(vec2 coords, vec4 mask, float scale, inout vec2 uv) {
    float rand = u_rng;
    coords.x += int(random(rand) * (u_fuzziness * 2 + 1) - u_fuzziness) / float(u_width);
    coords.y += int(random(rand) * (u_fuzziness * 2 + 1) - u_fuzziness) / float(u_height);
    vec2 center = vec2(0.5, 0.5);
    vec2 point = coords - center;
    float radius = length(point);
    vec2 distortion = point * ((1.0 + u_distortion_amount * radius * radius) / (1.0 - scale)) / ((0.25 * u_distortion_amount) + 1);
    uv = distortion + center;
    if (uv.x < 0 || uv.y < 0 || uv.x > 1 || uv.y > 1) return vec4(0, 0, 0, 1) * mask;
    return texture2D(u_texture, uv) * mask;
}

vec4 crt(vec4 color) {
    vec2 coords = v_coord;
    if (u_level_timer < 30) return vec4(0, 0, 0, 1);
    else if (u_level_timer < 35) {
        float t = u_level_timer - 30;
        coords.x = ((coords.x * 2 - 1) / (t / 5)) / 2 + 0.5;
        coords.y = ((coords.y * 2 - 1) / 0.02   ) / 2 + 0.5;
    }
    else if (u_level_timer < 40) {
        float t = u_level_timer - 35;
        coords.y = ((coords.y * 2 - 1) / (t / 5)) / 2 + 0.5;
    }
    vec2 uvR = vec2(0, 0);
    vec2 uvG = vec2(0, 0);
    vec2 uvB = vec2(0, 0);
    vec4 col = get(coords, vec4(1, 0, 0, 1), -u_chromatic_aberration, uvR) + get(coords, vec4(0, 1, 0, 1), 0, uvG) + get(coords, vec4(0, 0, 1, 1), u_chromatic_aberration, uvB);
    float multiplier = u_rng * u_flicker_strength + (1 - u_flicker_strength);
    if (uint(uvG.y * u_scanline_amount) % uint(2) == uint(1)) multiplier *= u_scanline_brightness;
    return col * multiplier;
}

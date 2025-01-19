#version 330

varying vec2 v_coord;
varying vec4 v_color;

uniform sampler2D u_texture;
uniform int u_timer;
uniform int u_width;
uniform int u_height;

uniform float u_noise_strength = 0.1;
uniform float u_wave_strength = 0.05;
uniform float u_num_waves = 2.5;
uniform float u_wave_speed = 0.001;

const float PI = 3.141592653589;

// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83#classic-perlin-noise

vec4 permute(vec4 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

vec2 fade(vec2 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float cnoise(vec2 P){
    vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
    vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
    Pi = mod(Pi, 289.0);
    vec4 ix = Pi.xzxz;
    vec4 iy = Pi.yyww;
    vec4 fx = Pf.xzxz;
    vec4 fy = Pf.yyww;
    vec4 i = permute(permute(ix) + iy);
    vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0;
    vec4 gy = abs(gx) - 0.5;
    vec4 tx = floor(gx + 0.5);
    gx = gx - tx;
    vec2 g00 = vec2(gx.x,gy.x);
    vec2 g10 = vec2(gx.y,gy.y);
    vec2 g01 = vec2(gx.z,gy.z);
    vec2 g11 = vec2(gx.w,gy.w);
    vec4 norm = 1.79284291400159 - 0.85373472095314 * vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
    g00 *= norm.x;
    g01 *= norm.y;
    g10 *= norm.z;
    g11 *= norm.w;
    float n00 = dot(g00, vec2(fx.x, fy.x));
    float n10 = dot(g10, vec2(fx.y, fy.y));
    float n01 = dot(g01, vec2(fx.z, fy.z));
    float n11 = dot(g11, vec2(fx.w, fy.w));
    vec2 fade_xy = fade(Pf.xy);
    vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
    float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
    return 2.3 * n_xy;
}

vec2 offset_point(vec2 point) {
    vec2 sample = point /*vec2(0.01, 0) * u_timer + vec2(0, sin(u_timer / 256.0))*/;
    return vec2(cnoise(sample), cnoise(sample + vec2(69 + u_timer / 256.0, 420 + u_timer / 512.0)));
}

void main() {
    vec2 pos = v_coord + offset_point(offset_point(v_coord * 8) * 8) * u_noise_strength;
    pos.x += sin((v_coord.y + u_timer * u_wave_speed) * 2 * PI * u_num_waves) * u_wave_strength;
    gl_FragColor = texture2D(u_texture, pos) * v_color;
}

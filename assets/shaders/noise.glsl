vec4 noise(vec4 color) {
    float rand = u_rng;
    if (random(rand) < 0.5) discard;
    else return color;
}

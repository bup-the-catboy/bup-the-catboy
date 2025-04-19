void main() {
    float rand = u_rng;
    if (random(rand) < 0.5) discard;
    else gl_FragColor = texture2D(u_texture, v_coord) * v_color;
}

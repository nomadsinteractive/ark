vec3 hdr_gamma_correct(vec3 color)
{
    return pow(color, vec3(1.0 / 2.2));
}

vec3 hdr_tone_mapping(vec3 color)
{
    return hdr_gamma_correct(color / (color + vec3(1.0)));
}

vec3 hdr_exposure_tone_mapping(vec3 color, float exposure)
{
    return hdr_gamma_correct(vec3(1.0) - exp(-color * exposure));
}

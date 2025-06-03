#version 330

uniform float time;
uniform vec2 resolution;

out vec4 fragColor;

// --- Bruit basique
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float smoothNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = noise(i);
    float b = noise(i + vec2(1.0, 0.0));
    float c = noise(i + vec2(0.0, 1.0));
    float d = noise(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

// --- Bruit fractal pour texture liquide
float fbm(vec2 p) {
    float value = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; i++) {
        value += amp * smoothNoise(p);
        p *= 2.0;
        amp *= 0.5;
    }
    return value;
}

// --- Onde organique simulant un effleurement
float expandingRipple(vec2 uv, float t) {
    vec2 source = vec2(
        fbm(vec2(t * 0.1, 0.0)) * 0.6,
        fbm(vec2(0.0, t * 0.1)) * 0.6
    );

    float dist = distance(uv, source);
    float wave = sin(dist * 50.0 - t * 5.0 + fbm(uv * 8.0 + t * 0.8) * 3.0);
    float attenuation = exp(-dist * 6.0) * exp(-t * 0.02);
    return wave * attenuation * 1.4;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float t = time;

    // Texture liquide
    vec2 p = uv * 3.0;
    float liquid = fbm(p + vec2(t * 0.1, t * 0.15)) * 0.8;

    // Onde fluide
    float ripple = expandingRipple(uv, t);
    float displacement = liquid + ripple;

    // Normale de surface simulée
    vec3 normal = normalize(vec3(displacement * 1.5, sin(t + displacement * 10.0) * 0.5, 1.0));

    // Spot lumineux centré
    vec2 lightPos = vec2(0.0, 0.0); // Spot au centre
    float spotRadius = 1.0;
    float d = distance(uv, lightPos);
    float spotFalloff = smoothstep(spotRadius, 0.0, d); // 1.0 au centre, 0.0 en dehors

    // Lumière directionnelle
    vec3 lightDir = normalize(vec3(lightPos - uv, 1.0));
    float shade = clamp(dot(normal, lightDir), 0.0, 1.0);
    float finalLight = shade * spotFalloff;

    // Couleur du métal liquide
    vec3 baseColor = mix(vec3(0.05, 0.05, 0.08), vec3(0.8, 0.9, 1.0), finalLight);
    vec3 tint = vec3(0.2, 0.5, 1.0) * pow(finalLight, 5.0);

    // --- Grain statique appliqué à toute l’image
    float grain = noise(gl_FragCoord.xy * 0.5); // Densité du grain
    grain = (grain - 0.5) * 0.05; // Intensité

    vec3 finalColor = baseColor + tint + vec3(grain);

    fragColor = vec4(finalColor, 1.0);
}


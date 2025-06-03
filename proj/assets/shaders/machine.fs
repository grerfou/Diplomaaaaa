#version 330 core

uniform float time;
uniform vec2 resolution;
uniform vec2 corners[4]; // TL, TR, BR, BL
uniform vec2 offset;

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

// Calcul bilinéaire UV locale dans le quadrilatère
vec2 getUV(vec2 fragCoord) {
    float minX = min(min(corners[0].x, corners[1].x), min(corners[2].x, corners[3].x));
    float maxX = max(max(corners[0].x, corners[1].x), max(corners[2].x, corners[3].x));
    float minY = min(min(corners[0].y, corners[1].y), min(corners[2].y, corners[3].y));
    float maxY = max(max(corners[0].y, corners[1].y), max(corners[2].y, corners[3].y));

    float xNorm = (fragCoord.x - minX) / (maxX - minX);
    float yNorm = (fragCoord.y - minY) / (maxY - minY);

    vec2 top = mix(corners[0], corners[1], xNorm);
    vec2 bottom = mix(corners[3], corners[2], xNorm);
    vec2 pos = mix(top, bottom, yNorm);

    return vec2(xNorm, yNorm);
}

void main() {
    vec2 fragCoord = gl_FragCoord.xy;

    // Calcul UV bilinéaire
    vec2 uv = getUV(fragCoord);

    // On ignore les fragments hors quadrilatère (marge 0.01 pour sécurité)
    if (uv.x < -0.01 || uv.x > 1.01 || uv.y < -0.01 || uv.y > 1.01) discard;

    // Recentrer UV dans [-1,1]
    vec2 centeredUV = uv * 2.0 - 1.0;
    centeredUV.x *= resolution.x / resolution.y;

    // Appliquer l'offset en coordonnées centrées
    vec2 offsetNormalized = offset / resolution;
    offsetNormalized *= 2.0;
    offsetNormalized.x *= resolution.x / resolution.y;
    centeredUV += offsetNormalized;

    float t = time;

    // Texture liquide
    vec2 p = centeredUV * 3.0;
    float liquid = fbm(p + vec2(t * 0.1, t * 0.15)) * 0.8;

    // Onde fluide
    float ripple = expandingRipple(centeredUV, t);
    float displacement = liquid + ripple;

    // Normale de surface simulée
    vec3 normal = normalize(vec3(displacement * 1.5, sin(t + displacement * 10.0) * 0.5, 1.0));

    // Spot lumineux centré
    vec2 lightPos = vec2(0.0, 0.0);
    float spotRadius = 1.0;
    float d = distance(centeredUV, lightPos);
    float spotFalloff = smoothstep(spotRadius, 0.0, d);

    // Lumière directionnelle
    vec3 lightDir = normalize(vec3(lightPos - centeredUV, 1.0));
    float shade = clamp(dot(normal, lightDir), 0.0, 1.0);
    float finalLight = shade * spotFalloff;

    // Couleur du métal liquide
    vec3 baseColor = mix(vec3(0.05, 0.05, 0.08), vec3(0.8, 0.9, 1.0), finalLight);
    vec3 tint = vec3(0.2, 0.5, 1.0) * pow(finalLight, 5.0);

    // Grain statique
    float grain = noise(fragCoord * 0.5);
    grain = (grain - 0.5) * 0.05;

    vec3 finalColor = baseColor + tint + vec3(grain);

    fragColor = vec4(finalColor, 1.0);
}


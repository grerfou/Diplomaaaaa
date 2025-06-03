#version 330 core

uniform float time;
uniform vec2 resolution;
uniform vec2 corners[4]; // [0] = BL, [1] = BR, [2] = TR, [3] = TL
uniform vec2 offset;
out vec4 fragColor;

// --- Fonctions auxiliaires (noise, fbm, getNormal, etc) ---

float noise(vec2 p) {
    return fract(sin(dot(p ,vec2(127.1,311.7))) * 43758.5453123);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.6;
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(p);
        p *= 1.0;
        amplitude *= 0.5;
    }
    return value;
}

vec3 getNormal(vec2 uv, vec2 center, float radius) {
    vec2 diff = uv - center;
    float dist = length(diff);
    float z = sqrt(max(0.0, radius * radius - dist * dist));
    return normalize(vec3(diff, z));
}

float wavePattern(vec2 uv, float time) {
    float wave = sin((uv.y - time * 0.3) * 20.0 + sin(uv.x * 5.0 + time)) * 0.5 + 0.5;
    wave *= exp(-pow((uv.y - 0.5) * 4.0, 2.0));
    return wave;
}

float smoothEdge(float dist, float radius, float blur) {
    return smoothstep(radius, radius - blur, dist);
}

float grain(vec2 uv, float time) {
    float g = noise(uv * resolution.xy * 0.5 + time * 0.5);
    return g * 0.05;
}

void main() {

    vec2 fragCoord = gl_FragCoord.xy;
    vec2 st = fragCoord / resolution;

    vec2 bottom = mix(corners[0], corners[1], st.x);
    vec2 top = mix(corners[3], corners[2], st.x);

    vec2 uvWarped = mix(bottom, top, st.y);
    uvWarped += offset; // Appliquer l'offset ici




/*
    vec2 st = gl_FragCoord.xy / resolution.xy;
    // Interpolation bilinéaire entre les 4 coins comme dans ton exemple
    vec2 bottom = mix(corners[0], corners[1], st.x);
    vec2 top = mix(corners[3], corners[2], st.x);
    vec2 uvWarped = mix(bottom, top, st.y);
*/
    // Normaliser uvWarped entre [0,1] selon coin BL et TR
    // ATTENTION: Pour que ça marche, corners doivent approx. former un rectangle
    vec2 uv = (uvWarped - corners[0]) / (corners[2] - corners[0]);

    // Correction ratio d’aspect
    uv.x *= resolution.x / resolution.y;

    vec2 center = vec2(0.5 * resolution.x / resolution.y, 0.5);
    float radius = 0.30;
    float dist = distance(uv, center);

    vec3 background = vec3(0.01, 0.01, 0.015) * (1.0 - uv.y);
    vec3 color = background;

    float wave = wavePattern(uv, time);
    vec3 waveColor = vec3(0.9 + 0.2 * sin(time + uv.y * 20.0), 0.2 + 0.8 * sin(time + uv.y * 25.0), 0.6 + 0.4 * sin(time + uv.y * 30.0));

    float edgeFade = smoothEdge(dist, radius, 0.015);

    if (dist < radius) {
        vec3 N = getNormal(uv, center, radius);
        vec3 I = vec3(0.0, 0.0, -1.0);
        float fresnel = pow(1.0 - dot(N, I), 3.0);

        float n = fbm(uv * 100.0 + time * 0.8);
        vec3 glassColor = vec3(0.15, 0.18, 0.22) + 0.08 * n;

        float etaR = 1.0 / 1.01;
        float etaG = 1.0 / 1.02;
        float etaB = 1.0 / 1.03;

        vec3 refractDirR = refract(I, N, etaR);
        vec3 refractDirG = refract(I, N, etaG);
        vec3 refractDirB = refract(I, N, etaB);

        float offset = 0.02;
        vec2 uvR = uv + refractDirR.xy * offset;
        vec2 uvG = uv + refractDirG.xy * offset;
        vec2 uvB = uv + refractDirB.xy * offset;

        float blend = (4.0 - dist / radius);
        vec3 dispersion = vec3(
            fbm(uvR * 60.0 + time * 0.4),
            fbm(uvG * 60.0 + time * 0.4),
            fbm(uvB * 60.0 + time * 0.4)
        );

        float pulse = sin(time * 2.0 + dist * 20.0) * 0.5 + 0.5;
        float organicFactor = fbm(uv * 30.0 + vec2(time * 0.5, -time * 0.3));

        vec3 waveInside = waveColor * wave * (0.4 + 0.6 * organicFactor) * pulse;
        vec3 refracted = waveInside * blend;

        color = mix(glassColor, refracted + glassColor + 0.8 * dispersion, 0.85);
        color = mix(color, vec3(1.0), fresnel * 0.1);
        color = mix(background, color, edgeFade);
    }

    color += grain(uv, time);

    fragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}


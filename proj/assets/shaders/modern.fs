#version 330 core

uniform float time;
uniform vec2 resolution;
uniform vec2 corners[4]; // [0]=BL, [1]=BR, [2]=TR, [3]=TL
uniform vec2 offset;
out vec4 fragColor;

// --- Fonctions auxiliaires ---

float noise(vec2 p) {
    return fract(sin(dot(p ,vec2(127.1,311.7))) * 43758.5453123);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.6;
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
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

float smoothEdge(float dist, float radius, float blur) {
    return smoothstep(radius, radius - blur, dist);
}

float grain(vec2 uv, float time) {
    return noise(uv * resolution.xy * 0.5 + time * 0.5) * 0.05;
}

vec3 rotatingHighlight(vec2 uv, vec3 normal, float time, float offset) {
    float angle = time + offset;
    vec3 lightDir = normalize(vec3(cos(angle), sin(angle), 1.0));
    float highlight = max(dot(normal, lightDir), 0.0);
    highlight = pow(highlight, 30.0 + 10.0 * noise(uv * 50.0 + time));
    float pulse = 0.5 + 0.5 * sin(time * 0.5 + offset + fbm(uv * 10.0));
    vec3 baseColor = vec3(1.2, 0.6, 0.3) * pulse;
    return baseColor * highlight;
}

vec3 clockNeedleHighlight(vec2 uv, vec2 center, float radius, float pingpongTime, float index, float glitchAmount) {
    vec2 rel = uv - center;
    float angle = atan(rel.y, rel.x);
    float dist = length(rel);

    float jitter = glitchAmount * sin(pingpongTime * 10.0 + index * 1.5) * 0.5;

    const float interval = 1.2;
    float maxAngle = 8.2831;

    float angleProgress = pingpongTime / interval;

    float spacing = 0.02;
    float needleAngle = angleProgress * maxAngle + jitter + index * spacing;

    float angleDist = abs(sin((angle - needleAngle) * 0.5));
    float needle = smoothstep(0.01, 0.003, angleDist);
    float radial = smoothstep(radius * 0.95, radius * 0.75, dist);
    float flicker = 0.6 + 0.4 * sin(pingpongTime * 5.0 + dist * 30.0 + index);
    vec3 needleColor = vec3(0.7, 0.85, 0.95) * flicker;

    return needleColor * needle * radial;
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
    // Interpolation bilinéaire entre les 4 coins (mapping déformé)
    vec2 bottom = mix(corners[0], corners[1], st.x);
    vec2 top = mix(corners[3], corners[2], st.x);
    vec2 uvWarped = mix(bottom, top, st.y);
*/
    // Normalisation uvWarped dans l’espace local (entre BL et TR)
    vec2 uv = (uvWarped - corners[0]) / (corners[2] - corners[0]);

    // Correction ratio d’aspect (pour garder forme non étirée)
    uv.x *= resolution.x / resolution.y;

    vec2 center = vec2(0.5 * resolution.x / resolution.y, 0.5);
    float radius = 0.35;
    float dist = distance(uv, center);

    vec3 background = vec3(0.01, 0.01, 0.015) * (1.0 - uv.y);
    vec3 color = background;

    float edgeFade = smoothEdge(dist, radius, 0.015);

    const float interval = 0.01;
    const float speed = 0.3;
    float pingpongTime = abs(fract(time * speed / interval) * 2.0 - 1.0) * interval;

    float glitchAmount = 0.1;

    if (dist < radius) {
        vec3 N = getNormal(uv, center, radius);
        vec3 I = vec3(0.0, 0.0, -1.0);
        float fresnel = pow(1.0 - dot(N, I), 3.0);

        float n = fbm(uv * 100.0 + time * 0.3);
        vec3 glassColor = vec3(0.08, 0.12, 0.16) + 0.06 * n;

        float etaR = 1.0 / 1.01;
        float etaG = 1.0 / 1.02;
        float etaB = 1.0 / 1.03;

        vec3 refractDirR = refract(I, N, etaR);
        vec3 refractDirG = refract(I, N, etaG);
        vec3 refractDirB = refract(I, N, etaB);

        float offset = 0.03 + 0.01 * sin(time * 0.2 + dist * 5.0);
        vec2 uvR = uv + refractDirR.xy * offset;
        vec2 uvG = uv + refractDirG.xy * offset;
        vec2 uvB = uv + refractDirB.xy * offset;

        vec3 dispersion = vec3(
            fbm(uvR * 30.0 + time * 0.2),
            fbm(uvG * 30.0 + time * 0.2),
            fbm(uvB * 30.0 + time * 0.2)
        );

        vec3 highlights = vec3(0.0);
        for (int i = 0; i < 6; i++) {
            float phase = float(i) * 0.8;
            highlights += rotatingHighlight(uv, N, time, phase);
        }

        vec3 gearColor = vec3(0.0);
        for (int i = 0; i < 30; i++) {
            gearColor += clockNeedleHighlight(uv, center, radius, pingpongTime, float(i), glitchAmount);
        }

        vec3 refracted = highlights * 1.2 + dispersion * 0.8 + gearColor * 1.2;

        color = mix(glassColor, refracted + glassColor, 0.9);
        color = mix(color, vec3(1.0), fresnel * 0.1);
        color = mix(background, color, edgeFade);
    }

    color += vec3(grain(uv, time));

    fragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}


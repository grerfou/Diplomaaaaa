#version 330 core

uniform vec2 resolution;
uniform float time;

uniform vec2 corners[4];  // TL, TR, BR, BL
uniform vec2 offset;
uniform float scale;

out vec4 FragColor;

//const float sheetWidth = 0.65;
const float sheetWidth = 0.95; // Scale !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const float edgeFade = 0.05;
const float grainAmount = 0.15;

float curve(float pos, float t) {
    float wave1 = 0.1 * sin(6.2831 * pos + t * 1.0);
    float wave2 = 0.06 * sin(12.566 * pos + t * 1.5 + 1.5);
    float wave3 = 0.04 * sin(18.849 * pos + t * 0.7 + 3.0);
    return wave1 + wave2 + wave3;
}

// Palette colors unchanged
vec3 palette(float t) {
    vec3 colors[6];
    colors[0] = vec3(1.0, 0.9, 0.5);
    colors[1] = vec3(1.0, 0.5, 0.1);
    colors[2] = vec3(0.8, 0.1, 0.0);
    colors[3] = vec3(0.3, 0.2, 0.7);
    colors[4] = vec3(0.1, 0.5, 0.9);
    colors[5] = vec3(1.0, 1.0, 1.0);
    float scaledT = mod(t * 6.0, 6.0);
    int index = int(floor(scaledT));
    float frac = fract(scaledT);
    vec3 c1 = colors[index % 6];
    vec3 c2 = colors[(index + 1) % 6];
    return mix(c1, c2, frac);
}

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Fonction bilinéaire qui calcule UV locaux selon le quadrilatère
vec2 getUV(vec2 fragCoord) {
    // Trouver bbox min/max
    float minX = min(min(corners[0].x, corners[1].x), min(corners[2].x, corners[3].x));
    float maxX = max(max(corners[0].x, corners[1].x), max(corners[2].x, corners[3].x));
    float minY = min(min(corners[0].y, corners[1].y), min(corners[2].y, corners[3].y));
    float maxY = max(max(corners[0].y, corners[1].y), max(corners[2].y, corners[3].y));

    float xNorm = (fragCoord.x - minX) / (maxX - minX);
    float yNorm = (fragCoord.y - minY) / (maxY - minY);

    vec2 top = mix(corners[0], corners[1], xNorm);
    vec2 bottom = mix(corners[3], corners[2], xNorm);
    vec2 pos = mix(top, bottom, yNorm);

    // On récupère les coordonnées relatives (xNorm,yNorm)
    return vec2(xNorm, yNorm);
}

void main() {
    // Calcul des UV locaux dans le quadrilatère
    vec2 uv = getUV(gl_FragCoord.xy);

    // Ne rien dessiner hors zone
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) discard;

    // Appliquer offset + scale autour du centre (0.5, 0.5)
    uv = ((uv - 0.5) + offset) * scale + 0.5;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) discard;

    // Aspect ratio à gérer (comme dans ton shader)
    uv.x *= resolution.x / resolution.y;

    vec2 diag = vec2(1.0, 1.0);
    float diagCoord = dot(uv, diag) / length(diag);
    float orthoCoord = dot(uv, vec2(-1.0, 1.0)) / length(vec2(-1.0, 1.0));

    diagCoord -= 0.5;
    orthoCoord -= 0.5;

    float curveOffset = curve(diagCoord, time);
    float distanceFromEdge = abs(orthoCoord - curveOffset);

    float mask = smoothstep(sheetWidth, sheetWidth - edgeFade, distanceFromEdge);
    float colorT = diagCoord + 0.5;

    vec3 baseColor = palette(colorT);

    vec2 lightDir = normalize(vec2(-0.7, 0.7));
    float epsilon = 0.001;
    float curvePrev = curve(diagCoord - epsilon, time);
    float curveNext = curve(diagCoord + epsilon, time);
    float slope = (curveNext - curvePrev) / (2.0 * epsilon);
    vec2 normal = normalize(vec2(-slope, 1.0));

    float diff = max(dot(normal, lightDir), 0.0);
    float ambient = 0.4;
    float lighting = ambient + diff * 0.8;
    lighting *= (1.0 - distanceFromEdge / sheetWidth);
    lighting = clamp(lighting, 0.0, 1.0);

    vec3 color = baseColor * lighting * mask;

    float grain = (rand(gl_FragCoord.xy + time) - 0.5) * grainAmount;
    color += grain;
    color = clamp(color, 0.0, 1.0);

    float alpha = mask * 0.7;

    FragColor = vec4(color, alpha);
}


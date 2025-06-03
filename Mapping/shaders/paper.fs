#version 330 core

uniform vec2 resolution;
uniform float time;

out vec4 FragColor;

const float sheetWidth = 0.65;
const float edgeFade = 0.05;

// Réglage du grain directement dans le shader
const float grainAmount = 0.15;  // Change cette valeur ici pour plus ou moins de grain

float curve(float pos, float t) {
    float wave1 = 0.1 * sin(6.2831 * pos + t * 1.0);
    float wave2 = 0.06 * sin(12.566 * pos + t * 1.5 + 1.5);
    float wave3 = 0.04 * sin(18.849 * pos + t * 0.7 + 3.0);
    return wave1 + wave2 + wave3;
}

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

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
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

    // Grain avec intensité fixe définie dans le shader
    float grain = (rand(gl_FragCoord.xy + time) - 0.5) * grainAmount;
    color += grain;
    color = clamp(color, 0.0, 1.0);

    float alpha = mask * 0.7;

    FragColor = vec4(color, alpha);
}


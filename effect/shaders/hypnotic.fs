#version 330 core
out vec4 finalColor;
uniform float time;
uniform vec2 resolution;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f*f*(3.0 - 2.0*f);
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b)* u.x * u.y;
}

float fbm(vec2 st) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for(int i = 0; i < 5; i++) {
        value += amplitude * noise(st * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float ring(float radius, float dist, float thickness) {
    return smoothstep(radius + thickness * 1.5, radius - thickness * 1.5, dist);
}

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    vec2 center = vec2(0.5);
    vec2 pos = uv - center;
    pos.x *= resolution.x / resolution.y;
    float dist = length(pos);

    vec3 backgroundColor = vec3(0.0);

    vec3 glowColor = vec3(0.85, 0.50, 0.18); // ocre/argile chaud

    //vec3 glowColor = vec3(0.93, 0.77, 0.56);  // couleur sable sahara chaude

    int ringsCount = 2;
    float minRadius = 0.05;
    float maxRadius = 0.35;
    float thickness = 0.006;

    float ringsLight = 0.0;

    for (int i = 0; i < ringsCount; i++) {
        float t = float(i) / float(ringsCount - 1);
        float radius_i = mix(minRadius, maxRadius, t);

        // vitesse fixée dans le shader : base entre 0.15 et 0.7, dépend de l'index
        float baseSpeed = 0.15  * fract(sin(float(i) * 12.9898) * 43758.5453);

        // sens alterné : pairs tournent dans un sens, impairs dans l'autre
        float dir = mod(float(i), 2.0) == 0.0 ? 1.0 : -1.0;

        float angle = atan(pos.y, pos.x);

        // noise pour variation subtile
        float noiseInput = float(i) * 5.0 + time * baseSpeed * dir;
        float noiseValue = fbm(vec2(noiseInput, dist * 10.0));
        
        float phase = time * baseSpeed * dir + noiseValue * 3.14159;

        float diffAngle = mod(angle - phase + 6.2831853, 6.2831853);

        float trailLength = 3.1415926 * 2.0;  // longue traînée lumineuse couvrant tout le cercle
        float fadeInStart = 0.0;
        float fadeInEnd = trailLength * 0.25;
        float fadeOutStart = trailLength * 0.75;
        float fadeOutEnd = trailLength;

        float intensity = smoothstep(fadeInStart, fadeInEnd, diffAngle) * smoothstep(fadeOutEnd, fadeOutStart, diffAngle);

        float ringIntensity = ring(radius_i, dist, thickness) * intensity;

        ringsLight += ringIntensity;
    }

    ringsLight = clamp(ringsLight, 0.0, 1.0);

    // Lumière orangée sable sahara, ajustée en intensité
    backgroundColor += glowColor * ringsLight * 1.3;

    // grain léger transparent sur tout l'écran
    float grainAmount = 0.15;
    float grain = random(uv * resolution.xy + time * 10.0);
    grain = (grain - 0.5) * grainAmount;
    backgroundColor += grain;

    backgroundColor = clamp(backgroundColor, 0.0, 1.0);

    finalColor = vec4(backgroundColor, 1.0);
}


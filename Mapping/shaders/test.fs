// Frag
#version 330

in vec2 fragUV;
out vec4 finalColor;

uniform vec2 topLeft;
uniform vec2 topRight;
uniform vec2 bottomLeft;
uniform vec2 bottomRight;

void main()
{
    vec2 top = mix(topLeft, topRight, fragUV.x);
    vec2 bottom = mix(bottomLeft, bottomRight, fragUV.x);
    vec2 pos = mix(top, bottom, fragUV.y);

    vec2 normPos = pos / vec2(800.0, 600.0);

    float dist = length(normPos - vec2(0.5, 0.5));
    float radius = 0.3;

    float alpha = smoothstep(radius, radius - 0.02, dist);

    vec3 bgColor = vec3(0.0);    // fond noir opaque
    vec3 circleColor = vec3(1.0); // blanc

    vec3 color = mix(bgColor, circleColor, alpha);

    finalColor = vec4(color, 1.0);
}


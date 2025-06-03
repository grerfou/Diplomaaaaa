// vert
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 fragUV;

void main()
{
    fragUV = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 1.0);
}


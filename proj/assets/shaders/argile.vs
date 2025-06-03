#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

// Uniforms pour la déformation du quad (passés depuis l’application)
uniform vec2 corners[4]; 

out vec2 uvWarped;
out vec2 vCorners[4];

void main()
{
    // Passe les coins au fragment shader
    for (int i = 0; i < 4; i++) {
        vCorners[i] = corners[i];
    }

    // Interpolation UV simple (si tu as déjà un système d’interpolation personnalisé, adapte ici)
    uvWarped = uv;

    // Position transformée directement sans déformation ici (ajuste si tu déformes au vertex)
    gl_Position = vec4(position, 1.0);
}


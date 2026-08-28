#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 Normal;
out vec3 Pos;
out vec2 TexCoords;
out vec4 PosLightSpace;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 modelMatrix;
uniform mat4 lightSpaceMatrix;


void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    Normal = modelMatrix * aNormal;

    vec4 worldPosition = model * vec4(aPos,1.0f);
    Pos = worldPosition.xyz;

    PosLightSpace = lightSpaceMatrix * vec4(Pos, 1.0);

    TexCoords = aTexCoords;
}
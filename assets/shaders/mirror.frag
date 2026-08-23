#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D mirrorTexture;

void main()
{
	FragColor = texture(mirrorTexture, vec2(1.0 - TexCoords.x, TexCoords.y));
}
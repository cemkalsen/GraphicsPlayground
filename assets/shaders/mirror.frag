#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D mirrorTexture;

uniform float exposure;

void main()
{

	float gamma = 2.2;

	vec3 col = texture(mirrorTexture, vec2(1.0 - TexCoords.x, TexCoords.y)).rgb;
	col = vec3(1.0) - exp(-max(col, vec3(0.0)) * exposure);
	FragColor = vec4(pow(col, vec3(1.0/gamma)), 1.0);
}
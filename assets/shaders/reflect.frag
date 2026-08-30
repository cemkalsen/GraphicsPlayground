#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 viewPos;
uniform samplerCube cubeMap;

uniform int refractMode;

void main()
{
	vec3 viewDir = normalize(Position - viewPos);
	vec3 texPos;

	if(refractMode == 1)
		texPos = refract(viewDir, normalize(Normal), 1.0/1.52);
	else
		texPos = reflect(viewDir, normalize(Normal));

	FragColor = texture(cubeMap, texPos);
}
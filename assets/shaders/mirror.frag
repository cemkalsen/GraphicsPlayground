#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D mirrorTexture;

void main()
{
	FragColor = texture(mirrorTexture, vec2(1.0 - TexCoords.x, TexCoords.y));
	   
	float gamma = 2.2;

	FragColor.rgb = max(FragColor.rgb, vec3(0.0));
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma)); 
}
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model; 
uniform mat3 modelMatrix;

out VS_OUT
{
	vec3 Normal;
	vec3 Position;
} vs_out;



layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};

void main()
{
	vs_out.Normal = modelMatrix * aNormal;
	vs_out.Position = vec3(model * vec4(aPos, 1.0)); 
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}
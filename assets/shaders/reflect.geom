#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 Normal;
	vec3 Position;
} gs_in[];

out vec3 Normal;
out vec3 Position;

uniform float time;

vec3 explode(vec3 position, vec3 normal)
{
	float magnitude = 2.0;
	vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude;
	return position +direction;
}

vec3 GetNormal()
{
	vec3 a = gs_in[1].Position - gs_in[0].Position;
	vec3 b = gs_in[2].Position - gs_in[0].Position;
	return normalize(cross(a,b));
}

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};


void main()
{


	vec3 normal = GetNormal();
	
    for(int i = 0; i < 3; ++i)
    {
        vec3 explodedPosition = explode(gs_in[i].Position, normal);

        Position = explodedPosition;
        Normal = gs_in[i].Normal;

        gl_Position = projection * view * vec4(explodedPosition, 1.0);

        EmitVertex();
    }

	EndPrimitive();
}

#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool blur;

uniform float exposure;
uniform float near_plane;
uniform float far_plane;

const float offset = 1.0f / 300.0f;


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{
    if(blur)
    {
    	vec2 offsets[9] = vec2[]
	    (
		    vec2(-offset,  offset),
            vec2( 0.0f,    offset), 
            vec2( offset,  offset), 
            vec2(-offset,  0.0f), 
            vec2( 0.0f,    0.0f),   
            vec2( offset,  0.0f),  
            vec2(-offset, -offset),
            vec2( 0.0f,   -offset),
            vec2( offset, -offset)   
	    );

        float kernel[9] = float[](
            1.0 / 16, 2.0 / 16, 1.0 / 16,
            2.0 / 16, 4.0 / 16, 2.0 / 16,
            1.0 / 16, 2.0 / 16, 1.0 / 16  
        );

        vec3 sampleTex[9];

        for(int i = 0; i < 9; ++i)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }

        vec3 col = vec3(0.0);

        for(int i = 0; i < 9; ++i)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    else
        FragColor = texture(screenTexture, TexCoords);

    float gamma = 2.2;
   
    vec3 hdrColor = FragColor.rgb;
    hdrColor = max(hdrColor, vec3(0.0));

    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));


    FragColor = vec4(mapped, 1.0); 
}
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool sharpen;
uniform bool gammaEnabled;

const float offset = 1.0f / 300.0f;


void main()
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

    float kernel[9] = float[]
    (
        -1,-1,-1,
        -1, 9, -1,
        -1,-1, -1
    );

    vec3 sampleTex[9];

    for(int i = 0; i < 9; ++i)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }

    vec3 col = vec3(0.0);

    for(int i = 0; i < 9; ++i)
        col += sampleTex[i] * kernel[i];

    if(sharpen)
	    FragColor = vec4(col, 1.0);
    else
        FragColor = texture(screenTexture, TexCoords);

    float gamma = 2.2;

    if(gammaEnabled)
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma)); 
}
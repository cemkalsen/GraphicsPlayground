#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Pos;
in vec2 TexCoords;
uniform vec3 viewPos;
uniform bool blinn;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform float shininess;


struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirLight dirLight;

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;


};
uniform SpotLight spotLight;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(lightDir, normal),0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = 0.0;
    if(blinn)
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess* 4.0);
    else
        spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);

    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
    vec3 diffuse = light.diffuse * texture(texture_diffuse1, TexCoords).rgb * diff;
    vec3 specular = light.specular * texture(texture_specular1, TexCoords).rgb * spec;

    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{

    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(lightDir, normal),0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = 0.0;
    if(blinn)
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess* 4.0);
    else
        spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
    vec3 diffuse = light.diffuse * texture(texture_diffuse1, TexCoords).rgb * diff;
    vec3 specular = light.specular * texture(texture_specular1, TexCoords).rgb * spec;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(lightDir, normal),0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = 0.0;
    if(blinn)
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess* 4.0);
    else
        spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta-light.outerCutOff) / epsilon,0.0,1.0);

    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
    vec3 diffuse = light.diffuse * texture(texture_diffuse1, TexCoords).rgb * diff;
    vec3 specular = light.specular * texture(texture_specular1, TexCoords).rgb * spec;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;

	return (2.0 * near * far) / (far + near - z * (far-near));
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - Pos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    //vec3 result = vec3(0.0f);

    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
        result += CalcPointLight(pointLights[i], norm, Pos, viewDir);

    //result += CalcSpotLight( spotLight, norm, Pos, viewDir);

    FragColor = vec4(result, 1.0f);
	
  //float depth = LinearizeDepth(gl_FragCoord.z) / far;
  //  FragColor = vec4(vec3(depth), 1.0);
}
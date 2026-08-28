#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Pos;
in vec2 TexCoords;
in vec4 PosLightSpace;

uniform vec3 viewPos;
uniform bool blinn;
uniform bool showShadow;

uniform sampler2D shadowMap;

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



float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDist = texture(shadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;


    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
 
    float shadow = 0.0;

    vec2 texelSize = 1.0f / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth  = texture(shadowMap, projCoords.xy + vec2(x,y)*texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    if(projCoords.z > 1.0)
        shadow = 0.0;

        shadow /= 9;

    return shadow;
}


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

    vec3 lighting = ambient + (1.0 - ShadowCalculation(PosLightSpace, lightDir, normal)) * (diffuse + specular); 

    return lighting;
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

    if(showShadow)
    {
      //float depth = LinearizeDepth(gl_FragCoord.z) / far;
      float shadowed = ShadowCalculation(PosLightSpace,normalize(-dirLight.direction),norm);
      FragColor = vec4(vec3(1.0 - shadowed), 1.0);
    }


}
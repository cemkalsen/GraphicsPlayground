#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Pos;
in vec2 TexCoords;

uniform vec3 viewPos;

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;

    float shininess;
};

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{

    vec3 ambient = vec3(texture(material.diffuse, TexCoords)).rgb * light.ambient;
    vec3 emission = vec3(texture(material.emission, TexCoords)).rgb;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - Pos);

    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = diff  * vec3(texture(material.diffuse, TexCoords)).rgb * light.diffuse;

    vec3 viewDir = normalize(viewPos - Pos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir,reflectDir),0.0f),material.shininess);

    vec3 specular = spec * vec3(texture(material.specular, TexCoords)).rgb * light.specular;


    vec3 result = ambient  + diffuse + specular + emission;
    FragColor = vec4(result , 1.0f);
}
#version 460

layout (location = 0) in vec2 texCoord0;
layout (location = 1) in vec3 normal0;
layout (location = 2) in vec3 FragPos;

layout (location = 0) out vec4 FragColor;

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

vec3 lightColor = vec3( 1.0, 0.4, 0.4 );

layout (binding = 1) uniform UBO_FS
{
	Light light;
	vec3 camPos;
	float shininess;
} ubo;

layout (binding = 0) uniform sampler2D diffuse;
layout (binding = 1) uniform sampler2D specular;

void main ()
{
	vec3 ambient = ubo.light.ambient * texture( diffuse, texCoord0 ).rgb;
	
	vec3 norm = normalize( normal0 );
	vec3 lightDir = normalize( ubo.light.position - FragPos );

	float diff = max( dot( norm, lightDir ), 0.0 );
	vec3 diffuse = ubo.light.diffuse * diff * texture( diffuse, texCoord0 ).rgb;
	
	vec3 viewDir = normalize( ubo.camPos - FragPos );
	vec3 reflectDir = reflect( -lightDir, norm );
	float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), ubo.shininess );
	vec3 specular = ubo.light.specular * spec * texture( specular, texCoord0 ).rgb;
	
	FragColor = vec4( ( ambient + diffuse + specular ), 1.0 );
}
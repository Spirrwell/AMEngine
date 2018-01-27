#version 460

//uniform vec3 camPos;

in vec2 texCoord0;
in vec3 normal0;
in vec3 FragPos;

out vec4 FragColor;

//varying vec2 texCoord0;

struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material material;

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

uniform vec3 camPos;

//vec3 lightPos = vec3( 2.0, 0.0, 1.0 );
vec3 lightColor = vec3( 1.0, 0.4, 0.4 );

void main ()
{
	//FragColor = texture2D( diffuse, texCoord0 );

	vec3 ambient = light.ambient * texture( material.diffuse, texCoord0 ).rgb;
	
	vec3 norm = normalize( normal0 );
	//vec3 lightDir = normalize( lightPos - FragPos );
	vec3 lightDir = normalize( light.position - FragPos );

	float diff = max( dot( norm, lightDir ), 0.0 );
	vec3 diffuse = light.diffuse * diff * texture( material.diffuse, texCoord0 ).rgb;
	
	//float specularStrength = 1.0;
	vec3 viewDir = normalize( camPos - FragPos );
	vec3 reflectDir = reflect( -lightDir, norm );
	float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
	vec3 specular = light.specular * spec * texture( material.specular, texCoord0 ).rgb;
	
	FragColor = vec4( ( ambient + diffuse + specular ), 1.0 );
	//FragColor = vec4( result, 1.0 );
	
	//gl_FragColor = texture2D( diffuse, texCoord0 );
	//gl_FragColor = vec4( texture2D( diffuse, texCoord0 ).r, texture2D( diffuseg, texCoord0 ).g, texture2D( diffuseb, texCoord0 ).b, 1.0 );
	//gl_FragColor = color0;
	//gl_FragColor = vec4( 1.0, 1.0, 1.0, 1.0 );
	//gl_FragColor = vec4( texture2D( diffusetwo, texCoord0 ).r, texture2D( diffuse, texCoord0 ).g, texture2D( diffuse, texCoord0 ).b, 1.0f );
}
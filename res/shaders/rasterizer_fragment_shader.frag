#version 330 core

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTextureCoords;

out vec4 fragColor;


//Textures
uniform sampler2D uTexture;

//Lighting stuff
uniform vec3 uCameraPosition;

vec3 lightPosition = vec3(10,10,0);

vec3 ambientColor = vec3(0.2,0.2,0.2);
float ambientStrength = 0.3f;

vec3 diffuseColor = vec3(0.8,0.8,0.8);

vec3 specularColor = vec3(1,1,1);
float shininess = 64.0f;





vec3 CalculateLighting(vec3 fragPosition, vec3 fragNormal)
{
	vec3 toLight = normalize(lightPosition - fragPosition);
	vec3 toCamera = normalize(uCameraPosition - fragPosition);

	vec3 ambientLight = ambientStrength * ambientColor;

	float diffuseStrength = max(dot(toLight,fragNormal),0);
	vec3 diffuseLight = diffuseStrength * diffuseColor;


	vec3 halfDir = normalize(toCamera + toLight);
	float specularStrength = pow(max(dot(halfDir,fragNormal),0.0f),shininess);
	vec3 specularLight = specularStrength * specularColor;

	return ambientLight + diffuseLight + specularLight;
}


void main()
{
	vec3 fragNormal = normalize(vNormal);
	vec3 fragPosition = vPosition;


	vec3 baseColor = vec3(texture(uTexture,vTextureCoords));
	vec3 shadedColor = CalculateLighting(fragPosition,fragNormal);
	fragColor = vec4(baseColor * shadedColor,1.0);
}


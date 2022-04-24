#version 330 core

in vec2 tex;
in vec3 nor;
in vec3 fragPos;

uniform sampler2D Texture1; // base texture
uniform sampler2D Texture2; // cloud
uniform sampler2D Texture3; // night light
uniform bool torchLight;

struct Lighting {    

	// light source
	vec3 position;
    vec3 direction;
	vec3 color;
  
	// camera
	vec3 camPos;

	// phong
    float ambientStrength;
    float specularStrength;
	float shininess;

	// attenuation
	float constant;
    float linear;
    float quadratic;

	// spot light
	float phi;		// inner cone
	float gamma;	// outer cone
};
uniform Lighting light[2];

out vec4 fragCol;

// prototype
float directionalIllumination(Lighting l, vec3 normals, vec3 fragPosition);
float positionalIllumination(Lighting l, vec3 normals, vec3 fragPosition);
float spotIllumination(Lighting l, vec3 normals, vec3 fragPosition);
float calculateAttenuation(Lighting l, vec3 fragPosition);
float positionalDarkness(Lighting l, vec3 normals, vec3 fragPosition);
float spotDarkness(Lighting l, vec3 normals, vec3 fragPosition);

void main()
{
	vec4 baseTexCol = texture(Texture1,tex);
	vec4 secondaryTexCol = texture(Texture2, tex);
	vec4 darkTexCol = texture(Texture3, tex);
	
	//float phong = directionalIllumination(lighting, nor, fragPos);
	//float phong = spotIllumination(lighting, nor, fragPos);
	float phong = positionalIllumination(light[0], nor, fragPos);
	float darkness = positionalDarkness(light[0], nor, fragPos);
	if (torchLight) 
	{
		phong += spotIllumination(light[1], nor, fragPos);
		darkness *= spotDarkness(light[1], nor, fragPos);
	}

	vec4 baseColor = phong * (baseTexCol + secondaryTexCol) * vec4(light[0].color, 1.f);
	vec4 darkColor = darkness * darkTexCol;

	fragCol = baseColor + darkColor;
}

float directionalIllumination(Lighting l, vec3 normals, vec3 fragPosition)
{
	// clean input
	vec3 norm = normalize(normals);
	vec3 toLightDir = -normalize(l.direction);

	// calculate diffuse
	float diffuse = max( dot(norm, toLightDir), 0.0);

	// calcualte specular
	vec3 toCamDir = normalize(l.camPos - fragPosition);
	vec3 refDir = reflect(-toLightDir, norm);
	float specular = pow(max(dot(toCamDir, refDir), 0.0), l.shininess) * l.specularStrength;

	return l.ambientStrength + diffuse + specular;
}

float positionalIllumination(Lighting l, vec3 normals, vec3 fragPosition)
{	
	// clean input
	vec3 norm = normalize(normals);
	vec3 toLightDir = normalize(l.position - fragPosition);

	// calculate diffuse
	float diffuse = max( dot(norm, toLightDir), 0.0);

	// calcualte specular
	vec3 toCamDir = normalize(l.camPos - fragPosition);
	vec3 refDir = reflect(-toLightDir, norm);
	float specular = pow(max(dot(toCamDir, refDir), 0.0), l.shininess) * l.specularStrength;

	float phong = l.ambientStrength + diffuse + specular;
	float attenuation = calculateAttenuation(l, fragPosition);
	return phong * attenuation;
}

float positionalDarkness(Lighting l, vec3 normals, vec3 fragPosition)
{
	// clean input
	vec3 norm = normalize(normals);
	vec3 toLightDir = normalize(l.position - fragPosition);

	// calculate diffuse 
	// it will be higher if the normal is facing away from the light
	// and become 0 at the intersection of sun facing normals and shaded normals
	float darkness = min(dot(norm, toLightDir), 0);
	darkness = abs(darkness);

	return darkness;
}

float spotIllumination(Lighting l, vec3 normals, vec3 fragPosition)
{
	vec3 fromLightDir = -normalize(l.position - fragPosition); // direction of fragment to light position
	vec3 spotDir = normalize(l.direction); // direction of spot light center vector

	float cosTheta = dot(spotDir, fromLightDir);
	float cosPhi = cos(radians(l.phi));
	float cosGamma = cos(radians(l.gamma));
	float epsilon = cosPhi - cosGamma;
	//float intensity = clamp((cosTheta - cosGamma) / epsilon, 0.0, 1.0); 
	float intensity = smoothstep(0.0, 1.0, (cosTheta - cosGamma) / epsilon);

//	if(cosTheta > cosPhi)
//	{
//		return positionalIllumination(l, normals, fragPosition);
//	}
//	else
//	{
//		return l.ambientStrength * calculateAttenuation(l, fragPosition);
//	}

	return positionalIllumination(l, normals, fragPosition) * intensity;
}

float spotDarkness(Lighting l, vec3 normals, vec3 fragPosition)
{
	vec3 fromLightDir = -normalize(l.position - fragPosition); // direction of fragment to light position
	vec3 spotDir = normalize(l.direction); // direction of spot light center vector

	float cosTheta = dot(spotDir, fromLightDir);
	float cosPhi = cos(radians(l.phi));
	float cosGamma = cos(radians(l.gamma));
	float epsilon = cosPhi - cosGamma;
	float intensity = smoothstep(0.0, 1.0, (cosTheta - cosGamma) / epsilon);

	return 1 - intensity;
}

float calculateAttenuation(Lighting l, vec3 fragPosition)
{
	float dist = length(l.position - fragPosition);
	return 1/(l.constant + (l.linear * dist) + (l.quadratic * pow(dist, 2)));
}
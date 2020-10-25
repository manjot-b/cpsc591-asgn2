#version 330 core

in vec3 surfaceNormal;
in vec3 lightToFrag;

uniform vec3 color;
uniform vec3 blue;
uniform vec3 yellow;
uniform float coolIntensity;
uniform float warmIntensity;
uniform vec3 lightColor;
uniform vec3 toCamera;

out vec4 fragColor;

void main()
{ 
	// GOOCH LIGHTING
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightToFrag = normalize(lightToFrag);
	float blend = (1 + dot(unitNormal, unitLightToFrag)) * 0.5f;
	vec3 cool = blend * (blue + coolIntensity * color);
	vec3 warm = (1 - blend) * (yellow + warmIntensity * color);
	vec3 gooch = cool + warm;
	
	// SPECULAR LIGHTING
	float shininess = 32.0f;
	float specularCoeff = 0.5f;
	vec3 unitToCamera = normalize(toCamera);
	//vec3 reflectedDir = unitLightToFrag - 2 * max(dot(unitLightToFrag, unitNormal), 0) * unitNormal;
	vec3 reflectedDir = 2 * dot(-unitLightToFrag, unitNormal) * unitNormal + unitLightToFrag;
	reflectedDir = normalize(reflectedDir);
	float specularFactor = max(dot(reflectedDir, unitToCamera), 0);
	float dampedFactor = pow(specularFactor, shininess);
	vec3 specular = dampedFactor * specularCoeff * lightColor;

	vec3 finalColor = gooch + specular;

	fragColor = vec4(finalColor, 1.0f);
}

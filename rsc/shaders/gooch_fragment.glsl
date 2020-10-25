#version 330 core

in vec3 surfaceNormal;
in vec3 lightToFrag;

uniform vec3 color;
uniform vec3 blue;
uniform vec3 yellow;
uniform float coolIntensity;
uniform float warmIntensity;
uniform vec3 lightColor;

out vec4 fragColor;

void main()
{ 
	// GOOCH DIFFUSE LIGHTING
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightToFrag = normalize(lightToFrag);
	float diffBrightness = (1 + dot(unitNormal, unitLightToFrag)) * 0.5f;
	vec3 cool = diffBrightness * (blue + coolIntensity * color);
	vec3 warm = (1 - diffBrightness) * (yellow + warmIntensity * color);
	
	vec3 finalColor = cool + warm;

	fragColor = vec4(finalColor, 1.0f);
}

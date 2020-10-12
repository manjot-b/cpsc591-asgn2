#version 330 core

in vec4 vertexColor;
uniform vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, vertexColor.a);
}

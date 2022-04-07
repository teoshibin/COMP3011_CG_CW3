#version 330 core

in vec3 col;

out vec4 fragCol;

void main()
{
	fragCol = vec4(col, 1.f);
}

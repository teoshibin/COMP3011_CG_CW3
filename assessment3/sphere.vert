#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTex;
layout(location = 2) in vec3 aNor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 tex;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.f);
	tex = aTex.xy;
}

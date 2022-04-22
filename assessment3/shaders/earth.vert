#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aTex;
layout(location = 2) in vec3 aNor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 tex;
out vec3 nor;
out vec3 fragPos;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.f);
	tex = aTex.xy;
	fragPos = vec3(model * vec4(aPos, 1.f)); // world space position
	nor = mat3(transpose(inverse(model))) * aNor; // to fix non uniform scaling
}

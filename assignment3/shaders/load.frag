#version 330 core

in vec2 tex;
uniform sampler2D Texture;
out vec4 fragCol;

void main()
{
	fragCol = texture(Texture,tex);
}

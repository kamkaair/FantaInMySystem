#version 330 core
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_texCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 texCoord;

void main()
{
    texCoord = in_texCoord;
	gl_Position = projection * view * model * vec4(in_position, 1.0);
}
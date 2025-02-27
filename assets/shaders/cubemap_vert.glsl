#version 330 core
layout (location = 0) in vec3 in_position;

out vec3 worldPos;

uniform mat4 projection;
uniform mat4 view;

void main() {
    worldPos = in_position;
    gl_Position =  projection * view * vec4(worldPos, 1.0);
}
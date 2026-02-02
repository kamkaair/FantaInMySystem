#version 330 core
layout (location = 0) in vec3 in_position;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPos;

void main()
{
    localPos = in_position;

	//mat4 rotView = mat4(mat3(view)); // Remove translation from the view matrix
	// "projection * view" instead of "projection * rotView", because we do this in renderSkybox()
	vec4 clipPos = projection * view * vec4(localPos, 1.0);

	gl_Position = clipPos.xyww;
}
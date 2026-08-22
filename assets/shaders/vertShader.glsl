	#version 330 core

	out vec3 fragPos;
	out vec3 normal;
	out vec2 texCoord;
	out vec4 fragPosLightSpace;

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	layout (location = 2) in vec3 in_normal;
	layout (location = 3) in vec4 in_fragPosLightSpace;

	uniform mat4 VP;
	uniform mat4 M;
	uniform mat4 lightMatrix;
	
	void main() {
		fragPos = vec3(M * vec4(in_position, 1.0));
		normal = mat3(transpose(inverse(M))) * in_normal;
		//normal = mat3(M) * in_normal;
		texCoord = in_texCoord;
		fragPosLightSpace = lightMatrix * vec4(fragPos, 1.0);
		
		gl_Position = VP * vec4(fragPos, 1.0);
	};
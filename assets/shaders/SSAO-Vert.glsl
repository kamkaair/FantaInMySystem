	#version 330 core

	out vec3 fragPosIn;
	out vec2 texCoords;
	out vec3 normalIn;

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	layout (location = 2) in vec3 in_normal;
	
	void main()
	{
		fragPosIn = in_position;
		texCoords = in_texCoord;
		normalIn = in_normal;
		gl_Position = vec4(in_position, 1.0);
	}
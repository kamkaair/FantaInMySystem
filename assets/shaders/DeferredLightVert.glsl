	#version 330 core
	
	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;

	out vec2 texCoord;
	
	void main()
	{		
		texCoord = in_texCoord;
		gl_Position = vec4(in_position, 1.0);
	};
	
	// uniform mat4 M;
	// uniform mat4 VP;
	
	// void main()
	// {
		// fragPos = vec3(M * vec4(in_position, 1.0));
		// normal = mat3(transpose(inverse(M))) * in_normal;
		// texCoord = in_texCoord;
		
		// gl_Position = VP * vec4(fragPos, 1.0);
	// };
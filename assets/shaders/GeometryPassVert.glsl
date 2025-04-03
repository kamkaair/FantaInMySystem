	#version 330 core
	
	out vec3 fragPos;
	out vec3 normal;
	out vec2 texCoord;

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	layout (location = 2) in vec3 in_normal;
	
	uniform mat4 VP;
	uniform mat4 M;
	
	void main()
	{
		fragPos = vec3(M * vec4(in_position, 1.0));
		normal = mat3(transpose(inverse(M))) * in_normal;
		texCoord = in_texCoord;
		
		gl_Position = VP * vec4(fragPos, 1.0);
	};
	// Incorrect MVP
	
	// uniform mat4 model;
	// uniform mat4 view;
	// uniform mat4 projection;

	// void main()
	// {
		// FragPos = vec3(model * vec4(aPos, 1.0));
		// TexCoords = aTexCoords;
		// Normal = mat3(transpose(inverse(model))) * aNormal;  
		
		// gl_Position = projection * view * vec4(FragPos, 1.0);
	// }
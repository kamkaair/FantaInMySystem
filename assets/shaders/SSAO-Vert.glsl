	#version 330 core

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	
	out vec2 texCoords;
	
	void main()
	{
	    texCoords = in_texCoord;
		gl_Position = vec4(in_position, 1.0);
	
		//fragPos = vec3(view * vec4(in_position, 1.0));
		//normal = normalize(mat3(view) * in_normal);
		//texCoords = in_texCoord;
		
		//gl_Position = projection * view * vec4(fragPos, 1.0);
	}
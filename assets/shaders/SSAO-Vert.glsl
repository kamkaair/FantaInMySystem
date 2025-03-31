	#version 330 core

	//out vec3 fragPos;
	out vec2 texCoords;
	//out vec3 normal;

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	//layout (location = 2) in vec3 in_normal;
	
	//uniform mat4 projection;
	//uniform mat4 view;
	
	void main()
	{
	    texCoords = in_texCoord;
		gl_Position = vec4(in_position, 1.0);
	
		//fragPos = vec3(view * vec4(in_position, 1.0));
		//normal = normalize(mat3(view) * in_normal);
		//texCoords = in_texCoord;
		
		//gl_Position = projection * view * vec4(fragPos, 1.0);
	}
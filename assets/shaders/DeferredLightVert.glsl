
	#version 330 core
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexCoords;
	layout (location = 2) in vec3 aNormal;

	out vec2 texCoord;
	out vec3 fragPos;
	out vec3 normal;
	
	void main()
	{
		// fragPos = vec3(M * vec4(aPos, 1.0));
		// normal = mat3(transpose(inverse(M))) * aNormal;
		// texCoord = aTexCoords;
		
		// gl_Position = VP * vec4(fragPos, 1.0);
		
		fragPos = aPos;
		texCoord = aTexCoords;
		normal = aNormal;
		gl_Position = vec4(aPos, 1.0);
	};
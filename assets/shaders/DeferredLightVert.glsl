	#version 330 core
	
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexCoords;
	layout (location = 2) in vec3 aNormal;

	out vec2 texCoord;
	out vec3 fragPos;
	out vec3 normal;
	
	void main()
	{		
		fragPos = aPos;
		texCoord = aTexCoords;
		normal = aNormal;
		gl_Position = vec4(aPos, 1.0);
	};
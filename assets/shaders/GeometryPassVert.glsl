	#version 330 core
	
	out vec3 fragPos;
	out vec3 normal;
	out vec2 texCoord;

	layout (location = 0) in vec3 in_position;
	layout (location = 1) in vec2 in_texCoord;
	layout (location = 2) in vec3 in_normal;
	
	uniform mat4 M;
	uniform mat4 VP;
	uniform mat4 V;
	
	void main()
	{
		mat4 MV = V * M;
		
		//fragPos and normal both in view-space
		fragPos = vec3(MV * vec4(in_position, 1.0));
		
		//mat3 normalMatrix = mat3(transpose(inverse(M)));
		//vec3 worldNormal = normalize(normalMatrix * in_normal);
		//normal = normalize(mat3(V) * worldNormal);
		
		normal = normalize(mat3(transpose(inverse(MV))) * in_normal);
		
		//normal = normalize(mat3(V * M) * in_normal);
		
		//mat4 mvN = transpose(inverse(MV));
		//normal = mat3(mvN) * in_normal;	
		
		texCoord = in_texCoord;
		
		//gl_Position = VP * vec4(vec3(M * vec4(in_position, 1.0)), 1.0);
		gl_Position = VP * M * vec4(in_position, 1.0); // Model → World → View
	};
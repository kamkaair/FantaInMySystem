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
		normal = normalize(mat3(transpose(inverse(MV))) * in_normal); // Experimented why transpose and inverse is done in this operation: non-uniform scaling operations screw up the normals, this is a way to fix that. After applying transp + inv, the normals should remain orthogonal to the tangent plane of the surface
		//normal = normalize(mat3(MV) * in_normal);
		texCoord = in_texCoord;
		
		gl_Position = VP * M * vec4(in_position, 1.0); // In clip space
	};
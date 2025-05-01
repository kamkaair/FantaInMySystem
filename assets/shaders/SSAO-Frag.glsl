	#version 330 core
	out float FragColor;

	in vec2 texCoords;

	uniform sampler2D texNoise;
	uniform sampler2D gPosition;
	uniform sampler2D gNormal;

	uniform vec3 samples[64];

	// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
	uniform int kernelSize = 64;
	uniform float radius = 0.5;
	uniform float bias = 0.025;

	// tile noise texture over screen based on screen dimensions divided by noise size
	uniform vec2 noiseScale = vec2(640.0/4.0, 480.0/4.0); 

	uniform mat4 projection;

	void main()
	{
		// get input for SSAO algorithm
		vec3 fragPos = texture(gPosition, texCoords).xyz;
		vec3 normal = normalize(texture(gNormal, texCoords).rgb);
		vec3 randomVec = normalize(texture(texNoise, texCoords * noiseScale).xyz);
		// create TBN change-of-basis matrix: from tangent-space to view-space
		vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 TBN = mat3(tangent, bitangent, normal);
		// iterate over the sample kernel and calculate occlusion factor
		float occlusion = 0.0;
		for(int i = 0; i < kernelSize; ++i)
		{
			vec3 sampleVec = TBN * samples[i];        // rotated sample vector
			vec3 samplePos = fragPos + sampleVec * radius; // move the sample point in view-space

			vec4 offset = vec4(samplePos, 1.0);
			offset = projection * offset;              // project sample position into clip-space
			offset.xyz /= offset.w;                    // perspective divide
			offset.xyz = offset.xyz * 0.5 + 0.5;        // to [0,1] screen space

			// if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
				// continue; // skip samples outside screen

			float sampleDepth = texture(gPosition, offset.xy).z; // get depth at sample

			float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
		}
		occlusion = 1.0 - (occlusion / kernelSize);
		FragColor = occlusion;
	}
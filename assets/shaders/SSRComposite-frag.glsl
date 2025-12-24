	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uLightingTex;   // lighting buffer
	uniform sampler2D uSSRTex;        // SSR reflection color
	uniform sampler2D uGExtra;        // (optional) roughness/metallic buffer
	uniform bool useRoughnessMask;

	void main()
	{
		vec3 lighting = texture(uLightingTex, texCoords).rgb;
		vec4 ssr = texture(uSSRTex, texCoords);
		float metallic = 1.0;
		float roughness = 0.0;

		if (useRoughnessMask) {
			vec2 mr = texture(uGExtra, texCoords).rg;
			metallic = mr.r;
			roughness = mr.g;
		}

		// Scale SSR by metallic and glossiness (1 - roughness)
		float mask = metallic * (1.0 - roughness);
		vec3 result = lighting + ssr.rgb * mask; // * mask

		FragColor = vec4(result, 1.0);
	}
	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uLightPassTex;
	uniform sampler2D uIndirectDiff;
	uniform sampler2D uIndirectSpec;
	uniform sampler2D uEmission;
	uniform sampler2D uSSR;
	
	uniform float HDRIExposure = 1.0f, HDRIContrast = 1.0f, FinalColorExposure = 1.0f, FinalColorContrast = 2.2f;
	uniform vec3 FinalColorHue = vec3(1.0);
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast) {
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}

	void main()
	{	
		vec3 lightPass  = texture(uLightPassTex, texCoords).rgb;	
		vec3 indirectDiff  = texture(uIndirectDiff, texCoords).rgb;	
		vec3 indirectSpec  = texture(uIndirectSpec, texCoords).rgb;
		vec3 emission  = texture(uEmission, texCoords).rgb;
		vec4 ssr = texture(uSSR, texCoords).rgba;
		
		indirectSpec = mix(indirectSpec.rgb, ssr.rgb, ssr.a);
		
		vec3 outColor = (lightPass + gammaCorrect(indirectDiff + indirectSpec, HDRIExposure, HDRIContrast)) + emission;
		if(outColor == vec3(0.0f))
			discard;
		outColor = vec3(outColor.r * FinalColorHue.r, outColor.g * FinalColorHue.g, outColor.b * FinalColorHue.b);
		
		FragColor = vec4(gammaCorrect(outColor, FinalColorExposure, FinalColorContrast), 1.0);
	}
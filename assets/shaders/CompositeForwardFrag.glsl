	#version 330 core
	out vec4 FragColor;

	in vec2 texCoord;

	uniform sampler2D uLightPassTex;
	uniform sampler2D uBloom;
	
	//uniform float HDRIExposure = 1.0f, HDRIContrast = 1.0f, FinalColorExposure = 1.0f, FinalColorContrast = 2.2f;
	//uniform vec3 HDRIHue = vec3(1.0), FinalColorHue = vec3(1.0);
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast) {
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}

	void main()
	{	
		vec3 directLighting  = texture(uLightPassTex, texCoord).rgb;	
		vec3 bloomBlur = texture(uBloom, texCoord).rgb;
		
		/*vec3 inDirectLighting = gammaCorrect(indirectDiff + indirectSpec, HDRIExposure, HDRIContrast);
		inDirectLighting = vec3(inDirectLighting.r * HDRIHue.r, inDirectLighting.g * HDRIHue.g, inDirectLighting.b * HDRIHue.b);	
		vec3 outColor = (directLighting + inDirectLighting) + emission;*/
			
		vec3 outColor = directLighting;
		if(outColor == vec3(0.0f))
			discard;

		// Color tweaking
		//color = gammaCorrect(color, FinalColorExposure, FinalColorContrast); // HDR tonemapping and gamma correct
		//color = vec3(color.r * FinalColorHue.r, color.g * FinalColorHue.g, color.b * FinalColorHue.b);
		
		outColor.rgb += bloomBlur.rgb;
		
		//FragColor = vec4(gammaCorrect(outColor, FinalColorExposure, FinalColorContrast), 1.0);
		FragColor = vec4(outColor, 1.0);
	}
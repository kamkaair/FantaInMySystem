	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uLightPassTex;
	//uniform sampler2D gEmission;
	
	//uniform float HdrExposure = 1.0f, HdrContrast = 1.0f;
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast) {
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}

	void main()
	{	
		vec3 lightPass  = texture(uLightPassTex, texCoords).rgb;	
		vec3 outColor = lightPass;
		
		FragColor = vec4(outColor, 1.0);
	}
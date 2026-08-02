	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uSSR;
	
	uniform sampler2D uDirectDiffuse;
	uniform sampler2D uDirectSpec;
	uniform sampler2D uIndirectDiffuse;
	uniform sampler2D uIndirectSpecFallback;
	
	//uniform sampler2D uDepth;
	//uniform samplerCube uSkybox;
	//uniform sampler2D uBackgroundTex;
	uniform sampler2D uTransTex;
	uniform sampler2D gEmission;
	uniform sampler2D uBackgroundTex;
	
	//uniform int backgroundMode;
	uniform float HdrExposure = 1.0f, HdrContrast = 1.0f;
	//uniform mat4 invProjection;
	//uniform mat4 invView;
	
	uniform bool useRoughnessMask;
	
	//const float alphaThreshold = 0.3;
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast){
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}

	void main()
	{
		vec4 transparentObjects = texture(uTransTex, texCoords).rgba;
		
		vec3 backgroundTexture = texture(uBackgroundTex, texCoords).rgb;
		vec4 ssr = texture(uSSR, texCoords);

		vec3 directDiffuse  = texture(uDirectDiffuse, texCoords).rgb;
		vec3 directSpecular = texture(uDirectSpec, texCoords).rgb;
		vec3 indirectDiffuse = texture(uIndirectDiffuse, texCoords).rgb;
		vec3 fallbackSpec = texture(uIndirectSpecFallback, texCoords).rgb;
		vec3 emission = texture(gEmission, texCoords).rgb;
		
		vec3 indirectSpecular = mix(fallbackSpec.rgb, ssr.rgb, ssr.a);
		//vec3 indirectSpecular = ssr.rgb + fallbackSpec * (1.0 - ssr.a);
		
		vec3 directLight = directDiffuse + directSpecular;
		vec3 inDirectLight = gammaCorrect((indirectDiffuse + indirectSpecular) * HdrExposure, 1.0f, HdrContrast);
		
		//vec3 color = (gammaCorrect(directLight, 1.0f, 1.0f)) + (gammaCorrect(inDirectLight, HdrExposure, HdrContrast)); 
		//vec3 color = backgroundTexture.rgb + (gammaCorrect(directLight + inDirectLight, 1.0f, 2.2f)) + emission;
		vec3 backgroundColor = backgroundTexture.rgb * (1.0 - transparentObjects.a) + transparentObjects.rgb;
		vec3 color = backgroundColor + (gammaCorrect(directLight + inDirectLight, 1.0f, 2.2f)) + emission;
		
		//vec3 outColor = mix(color.rgb, transparentObjects.rgb, transparentObjects.a);
		vec3 outColor = color.rgb * (1.0 - transparentObjects.a) + transparentObjects.rgb;
		
		//FragColor = vec4(0.0, depth, 0.0, 1.0);
		FragColor = vec4(outColor, 1.0);
	}
	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D uSSR;
	
	uniform sampler2D uDirectDiffuse;
	uniform sampler2D uDirectSpec;
	uniform sampler2D uIndirectDiffuse;
	uniform sampler2D uIndirectSpecFallback;
	
	uniform sampler2D uDepth;
	uniform samplerCube uSkybox;
	
	uniform mat4 invProjection;
	uniform mat4 invView;
	
	uniform bool useRoughnessMask;
	
	vec3 gammaCorrect(vec3 color){
		color = color / (color + vec3(1.0));
		color = pow(color, vec3(1.0 / 2.2));
		
		return color;
	}
	
	vec3 reconstructViewDir(vec2 uv, float depth)
	{
		vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 view = invProjection * clip;
		view /= view.w;
		vec4 world = invView * vec4(view.xyz, 0.0);
		return normalize(world.xyz);
	}

	void main()
	{

		//vec3 lighting = texture(uLightingTex, texCoords).rgb;
		//vec3 lightingDiff = texture(uLightingDiffuse, texCoords).rgb;
		float depth = texture(uDepth, texCoords).r;
		if(depth >= 0.9999) {		
			vec3 dir = reconstructViewDir(texCoords, depth);
			vec3 sky = texture(uSkybox, dir).rgb;
			FragColor = vec4(gammaCorrect(sky), 1.0);
			return;		
		}
		
		vec4 ssr = texture(uSSR, texCoords);
		float metallic = 1.0;
		float roughness = 0.0;

		vec3 directDiffuse  = texture(uDirectDiffuse, texCoords).rgb;
		vec3 directSpecular = texture(uDirectSpec, texCoords).rgb;
		vec3 indirectDiffuse = texture(uIndirectDiffuse, texCoords).rgb;
		vec3 fallbackSpec = texture(uIndirectSpecFallback, texCoords).rgb;
		
		vec3 indirectSpecular = mix(fallbackSpec.rgb, ssr.rgb, ssr.a);
		//vec3 indirectSpecular = ssr.rgb + fallbackSpec * (1.0 - ssr.a);
		
		vec3 color = directDiffuse + directSpecular + indirectDiffuse + indirectSpecular;
		
		FragColor = vec4(gammaCorrect(color), 1.0);
	}
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
	uniform sampler2D uBackgroundTex;
	uniform sampler2D uTransTex;
	uniform sampler2D gEmission;
	
	uniform int backgroundMode;
	uniform float HdrExposure = 1.0f, HdrContrast = 1.0f;
	uniform mat4 invProjection;
	uniform mat4 invView;
	
	uniform bool useRoughnessMask;
	
	const float alphaThreshold = 0.3;
	
	vec3 gammaCorrect(vec3 color, float exposure, float contrast){
		color = color / (color + vec3(1.0)) * exposure;
		color = pow(color, vec3(1.0 / contrast));
		
		return color;
	}
	
	vec3 reconstructViewDir(vec2 uv, float depth) {  // reconstruct view direction in world space
		// Reverse the the whole coordinate space to the beginning 5.Screen -> 4.NDC -> 3.Clip -> 2.View -> 1.World
		vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0); // Convert UV (0, 1) + depth (0, 1) to clip space. Clip is in (-1, 1)
		vec4 view = invProjection * clip; // Undo projection with InvProj * Clip (normally to clip Proj * View)
		view /= view.w; // divide view by it's w
		vec4 world = invView * vec4(view.xyz, 0.0); // Transform direction into world space, InvView * View (w = 0, because it's a direction not a position)
		return normalize(world.xyz);
	}

	void main()
	{
		vec4 transparentObjects = texture(uTransTex, texCoords).rgba;
	
		float depth = texture(uDepth, texCoords).r;
		
		// Depth check for rendering the cubemap background.
		if(depth >= 0.9999 && transparentObjects.a <= alphaThreshold) {  // Added alpha threshold to further select between background and transparent objects + eliminate those a != 0.0 pixels!
			vec3 dir, sky;
			switch (backgroundMode) {
			case 0:
				dir = reconstructViewDir(texCoords, depth);
				sky = texture(uSkybox, dir).rgb;
				break;
			case 1:
				sky = texture(uBackgroundTex, texCoords).rgb;
				break;
			default:
				dir = reconstructViewDir(texCoords, depth);
				sky = texture(uSkybox, dir).rgb;
			}		
			FragColor = vec4(gammaCorrect(sky, 1.0f, 1.0f), 1.0);
			return;		
		}
		
		vec4 ssr = texture(uSSR, texCoords);
		float metallic = 1.0;
		float roughness = 0.0;

		vec3 directDiffuse  = texture(uDirectDiffuse, texCoords).rgb;
		vec3 directSpecular = texture(uDirectSpec, texCoords).rgb;
		vec3 indirectDiffuse = texture(uIndirectDiffuse, texCoords).rgb;
		vec3 fallbackSpec = texture(uIndirectSpecFallback, texCoords).rgb;
		vec3 emission = texture(gEmission, texCoords).rgb;
		
		vec3 indirectSpecular = mix(fallbackSpec.rgb, ssr.rgb, ssr.a);
		//vec3 indirectSpecular = ssr.rgb + fallbackSpec * (1.0 - ssr.a);
		
		vec3 directLight = directDiffuse + directSpecular;
		vec3 inDirectLight = gammaCorrect((indirectDiffuse + indirectSpecular) * HdrExposure, 1.0f, 2.2f);
		
		//vec3 color = (gammaCorrect(directLight, 1.0f, 1.0f)) + (gammaCorrect(inDirectLight, HdrExposure, HdrContrast)); 
		vec3 color = directLight + inDirectLight; 
		
		//color = gammaCorrect(color); // Gamma correction before mixing transparent objects. The fragShader already has gamma correction
		
		vec3 outColor = (mix(color.rgb, transparentObjects.rgb, transparentObjects.a)) + emission;
		//vec3 color = indirectSpecular;
		
		FragColor = vec4(gammaCorrect(outColor, 1.0f, HdrContrast), 1.0);
	}
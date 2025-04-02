	// Fragment shader (lighting.frag)
	#version 330 core
	out vec4 FragColor;

	in vec2 TexCoords;

	uniform sampler2D gPosition;
	uniform sampler2D gNormal;
	uniform sampler2D gAlbedoSpec;
	//uniform sampler2D ssao;

	struct PointLight {
		vec3 position;
		vec3 color;
		float constant;
		float linear;
		float quadratic;
	};

	uniform PointLight pointLights[32];
	uniform int NUM_POINT_LIGHTS;
	uniform vec3 viewPos;

	void main()
	{             
		// Retrieve data from gbuffer
		vec3 FragPos = texture(gPosition, TexCoords).rgb;
		vec3 Normal = texture(gNormal, TexCoords).rgb;
		vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
		float Specular = texture(gAlbedoSpec, TexCoords).a;
		//float AmbientOcclusion = texture(ssao, TexCoords).r;
		
		// Then calculate lighting as usual
		//vec3 lighting = Diffuse * 0.1 * AmbientOcclusion; // hard-coded ambient component
		vec3 lighting = Diffuse * 0.1; // hard-coded ambient component
		vec3 viewDir = normalize(viewPos - FragPos);
		
		for(int i = 0; i < NUM_POINT_LIGHTS; ++i)
		{
			// Diffuse
			vec3 lightDir = normalize(pointLights[i].position - FragPos);
			vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLights[i].color;
			
			// Specular
			vec3 halfwayDir = normalize(lightDir + viewDir);  
			float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
			vec3 specular = pointLights[i].color * spec * Specular;
			
			// Attenuation
			float distance = length(pointLights[i].position - FragPos);
			float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * distance * distance);
			
			diffuse *= attenuation;
			specular *= attenuation;
			lighting += diffuse + specular;
		}
		
		FragColor = vec4(lighting, 1.0);
	}
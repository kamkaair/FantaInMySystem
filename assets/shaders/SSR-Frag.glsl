	#version 330 core
	out float FragColor;

	in vec2 texCoords;

	uniform sampler2D gPosition;
	uniform sampler2D gNormal;

	uniform mat4 projection;
	// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
	float maxDistance = 15;
	float resolution  = 0.3;
	int   steps       = 10;
	float thickness   = 0.5;

	void main()
	{
		// Probably not needed, since I've already got my own texCoords
		vec2 texSize = textureSize(gPosition, 0).xy;
		vec4 uv = vec4(0.0);
		//vec2 texCoord = texCoords.xy / texSize;
		
		// Necessary variables for SSR
		vec4 fragPos = texture(gPosition, texCoords);
		vec3 unitFragPos = normalize(fragPos.xyz);
		vec3 normal = normalize(texture(gNormal, texCoords).rgb);
		vec3 pivot = normalize(reflect(unitFragPos, normal)); // The reflected direction of fragPos
		
		vec4 startView = vec4(fragPos.xyz + (pivot * 0), 1;
		vec4 endView = vec4(fragPos.xyz + (pivot * maxDistance), 1);
		
		vec4 startFrag = startView;
		startFrag = projection * startFrag; 		// Project to screen space
		startFrag.xyz /= startFrag.w; 				// Perform the perspective divide
		startFrag.xy = startFrag.xy * 0.5 + 0.5; 	// Convert the screen-space XY coordinates to UV coordinates.
		startFrag.xy *= texSize; 					// Convert the UV coordinates to fragment/pixel coordnates.
		
		vec4 endFrag = startView;	// Do the same as before, but with the endFrag
		endFrag = projection * endFrag;
		endFrag.xyz /= endFrag.w;
		endFrag.xy = endFrag.xy * 0.5 + 0.5;
		endFrag.xy *= texSize;
		
		vec2 frag  = startFrag.xy;
		uv.xy = frag / texSize;
		
		float deltaX    = endFrag.x - startFrag.x;
		float deltaY    = endFrag.y - startFrag.y;
		
		float useX      = abs(deltaX) >= abs(deltaY) ? 1 : 0;
		float delta     = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);
		
		vec2  increment = vec2(deltaX, deltaY) / max(delta, 0.001);
		
		float search0 = 0;
		float search1 = 0;	
		
		int hit0 = 0;
		int hit1 = 0;
		
		float viewDistance = startView.y;
		float depth        = thickness;
		
	    for (i = 0; i < int(delta); ++i) {
			frag      += increment;
			uv.xy      = frag / texSize;
			positionTo = texture(positionTexture, uv.xy);
			
			search1 = mix( (frag.y - startFrag.y) / deltaY, (frag.x - startFrag.x) / deltaX, useX);
			
			viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
			depth        = viewDistance - positionTo.y;
			
			if (depth > 0 && depth < thickness) {
			  hit0 = 1;
			  break;
			} else {
			  search0 = search1;
			}
	    }
		
		search1 = search0 + ((search1 - search0) / 2);	
		
		steps *= hit0;

		for (i = 0; i < steps; ++i) {
			frag       = mix(startFrag.xy, endFrag.xy, search1);
			uv.xy      = frag / texSize;
			positionTo = texture(positionTexture, uv.xy);
			
			viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
			depth        = viewDistance - positionTo.y;
			
			if (depth > 0 && depth < thickness) {
			  hit1 = 1;
			  search1 = search0 + ((search1 - search0) / 2);
			} else {
			  float temp = search1;
			  search1 = search1 + ((search1 - search0) / 2);
			  search0 = temp;
			}
		}
		
		float visibility = hit1 * positionTo.w * ( 1 - max( dot(-unitPositionFrom, pivot), 0)) * ( 1 - clamp( depth / thickness, 0, 1)) 
		  * ( 1 - clamp(length(positionTo - positionFrom) / maxDistance, 0, 1)) * (uv.x < 0 || uv.x > 1 ? 0 : 1) * (uv.y < 0 || uv.y > 1 ? 0 : 1);
		  
		visibility = clamp(visibility, 0, 1);
		
		uv.ba = vec2(visibility);
		// create TBN change-of-basis matrix: from tangent-space to view-space
		// vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
		// vec3 bitangent = cross(normal, tangent);
		// mat3 TBN = mat3(tangent, bitangent, normal);
		// iterate over the sample kernel and calculate occlusion factor
		
		FragColor = uv;
	}
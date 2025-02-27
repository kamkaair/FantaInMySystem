#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D iconTex;

void main()
{		
    // vec3 envColor = texture(iconTex, WorldPos).rgb;
	vec4 texColor = texture(iconTex, texCoord);
	
	// If the texture has transparency, discard transparent fragments
    if (texColor.a < 0.1)
        discard;
    
    // // HDR tonemap and gamma correct
    // envColor = envColor / (envColor + vec3(1.0));
    // envColor = pow(envColor, vec3(1.0/2.2)); 
	
	vec3 envColor = vec3(1.0);
    
    FragColor = vec4(texColor.rgb, texColor.a);
}
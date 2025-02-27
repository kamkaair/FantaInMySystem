#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D backTexture;
uniform float backExposure = 1.0;
uniform float backContrast = 2.2;

void main()
{			
	vec4 backgroundColor = texture(backTexture, texCoord);
	
	// HDR tonemap and gamma correct
    backgroundColor = backgroundColor / (backgroundColor + vec4(1.0)) * backExposure;
    backgroundColor = pow(backgroundColor, vec4(1.0 / backContrast)); 
	
	//color = color / (color + vec3(1.0)) * HdrExposure;
	//color = pow(color, vec3(1.0 / HdrContrast));
    
    FragColor = vec4(backgroundColor.rgb, 1.0);
}
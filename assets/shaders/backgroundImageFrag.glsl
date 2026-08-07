#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D backTexture;
uniform float BackgroundExposure = 1.0, BackgroundContrast = 2.2;
uniform vec3 BackgroundHue = vec3(1.0);

void main() {
	vec4 backgroundColor = texture(backTexture, texCoord);
	
	// HDR tonemap and gamma correct
    backgroundColor = backgroundColor / (backgroundColor + vec4(1.0)) * BackgroundExposure;
    backgroundColor = pow(backgroundColor, vec4(1.0 / BackgroundContrast)); 
	
	backgroundColor = vec4(backgroundColor.r * BackgroundHue.r, backgroundColor.g * BackgroundHue.g, backgroundColor.b * BackgroundHue.b, backgroundColor.a);
	
	//color = color / (color + vec3(1.0)) * HdrExposure;
	//color = pow(color, vec3(1.0 / HdrContrast));
    
    FragColor = vec4(backgroundColor.rgba);
}
	#version 330 core
	out vec4 FragColor;

	in vec2 texCoords;

	uniform sampler2D ssrInput;
	uniform vec2 direction; // (1.0, 0.0) for horizontal, (0.0, 1.0) for vertical
	uniform int width = 640, height = 480;
	uniform float sigma = 4.0;

	const int KERNEL_RADIUS = 5;

	float gaussian(float x, float sigma) {
		return exp(- (x * x) / (2.0 * sigma * sigma)) / (2.0 * 3.14159265 * sigma * sigma);
	}

	void main() {
		int resolution = width * height;
		vec2 texOffset = direction / resolution; // per-pixel offset for blur direction
		vec3 result = texture(ssrInput, texCoords).rgb * gaussian(0.0, sigma);
		float weightSum = gaussian(0.0, sigma);

		for (int i = 1; i <= KERNEL_RADIUS; ++i) {
			float weight = gaussian(float(i), sigma);
			vec2 offset = texOffset * float(i);

			result += texture(ssrInput, texCoords + offset).rgb * weight;
			result += texture(ssrInput, texCoords - offset).rgb * weight;
			weightSum += 2.0 * weight;
		}

		result /= weightSum;
		FragColor = vec4(result, 1.0);
	}
#version 330 core
layout (location = 0) out vec4 outSSR;

in vec2 texCoords;

uniform sampler2D uSSRCurrent;	// Output of ssr-frag.glsl
uniform sampler2D uSSRHistory;	// Last frame history
uniform sampler2D depthMap;
uniform sampler2D gNormal;

//uniform mat4 invView;
//uniform mat4 prevView;

uniform mat4 invProjection;
uniform mat4 prevProjection;

uniform sampler2D uPrevDepth;
uniform sampler2D uPrevNormal;

uniform float near;
uniform float far;

//uniform float SCR_WIDTH;
//uniform float SCR_HEIGHT;

float LinearizeDepth(float d)
{
    float z = d * 2.0 - 1.0;
    //float near = 0.1;
    //float far  = 100.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    vec2 uv = texCoords;

    vec4 currSSR = texture(uSSRCurrent, uv);

    // Reconstruct current view position
    float depth = texture(depthMap, uv).r;
    vec3 ndc = vec3(uv * 2.0 - 1.0, depth * 2.0 - 1.0);
    vec4 viewPos = invProjection * vec4(ndc, 1.0);
    viewPos /= viewPos.w;

    // Reproject into previous frame
	/*
	vec4 worldPos = invView * viewPos;
	vec4 prevViewPos = prevView * worldPos;
	vec4 prevClip = prevProjection * prevViewPos;
	prevClip /= prevClip.w;
	
	vec2 prevUV = prevClip.xy * 0.5 + 0.5;
	*/
	
    vec4 prevClip = prevProjection * viewPos;
    prevClip /= prevClip.w;
    vec2 prevUV = prevClip.xy * 0.5 + 0.5;
	
    // Reject off-screen reprojection
    if (prevUV.x < 0.0 || prevUV.x > 1.0 ||
        prevUV.y < 0.0 || prevUV.y > 1.0) {
        outSSR = currSSR;
        return;
    }
	
	vec4 historySSR = texture(uSSRHistory, prevUV);
	
	float currLinear = LinearizeDepth(depth);
    float prevLinear = LinearizeDepth(texture(uPrevDepth, prevUV).r);
	
	bool depthMismatch = abs(currLinear - prevLinear) > 0.01;
	
	// Normal validation
    vec3 currNormal = normalize(texture(gNormal, uv).xyz);
    vec3 prevNormal = normalize(texture(uPrevNormal, prevUV).xyz);

    bool normalMismatch = dot(currNormal, prevNormal) < 0.9;

    // Reject, if either one is mismatched and reset
    if (depthMismatch || normalMismatch)
    {
        outSSR = currSSR;
        return;
    }

    // Confidence-based blending	
	float accumulationRate = 0.1;
	float valid = currSSR.a;

	vec3 blended = mix(historySSR.rgb, currSSR.rgb, accumulationRate * valid); // Accumulate when valid 
	float blendedAlpha = mix(historySSR.a, currSSR.a, accumulationRate); // Alpha
	
	outSSR = vec4(blended, blendedAlpha);

	/*
	float historyValid = historySSR.a;
	float currValid = currSSR.a;
	
	float blend = historyValid * currValid * 0.9;
	
	float alpha = 0.1;   // accumulation speed (lower = slower but smoother)
	vec3 blendedColor = mix(currSSR.rgb, historySSR.rgb, 1.0 - alpha);
	float blendedAlpha = currSSR.a;  // don't decay validity
	
    //vec3 blendedColor = mix(currSSR.rgb, historySSR.rgb, blend);
    //float blendedAlpha = max(currSSR.a, historyValid * 0.9);

    outSSR = vec4(blendedColor, blendedAlpha);
	//outSSR = vec4(currSSR.a, 0.0, 0.0, 1.0);
	*/
}

#version 330 core

layout (location = 0) out vec4 reflectionColor;

uniform sampler2D gNormal;
uniform sampler2D colorBuffer;
uniform sampler2D depthMap;

uniform float SCR_WIDTH;
uniform float SCR_HEIGHT;
uniform mat4 invProjection;
uniform mat4 projection;

uniform float far;
uniform float near;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

vec3 TraceRay_ViewSpace(vec3 rayOriginView, vec3 rayDirView)
{
    const int MAX_STEPS = 64;
    const float STEP_SIZE = 0.1;      // scene units
    const float THICKNESS = 0.15;      // hit tolerance

    vec3 rayPosView = rayOriginView;

    for (int i = 0; i < MAX_STEPS; ++i)
    {
        rayPosView += rayDirView * STEP_SIZE;

        // Project to clip space
        vec4 clip = projection * vec4(rayPosView, 1.0);
        if (clip.w <= 0.0)
            break;

        vec3 ndc = clip.xyz / clip.w;
        vec2 uv = ndc.xy * 0.5 + 0.5;

        // Screen bounds
        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            break;

        float sceneDepth = LinearizeDepth(texture(depthMap, uv).r);
        float rayDepth   = -rayPosView.z; // view space forward is -Z

        float depthDiff = rayDepth - sceneDepth;

        if (depthDiff > 0.0 && depthDiff < THICKNESS)
        {
            return texture(colorBuffer, uv).rgb;
        }
    }

    return vec3(0.0);
}

// The correct pipeline: View space -> March -> Project -> Sample depth -> Compare in view space
void main(){
	vec2 uv = gl_FragCoord.xy / vec2(SCR_WIDTH, SCR_HEIGHT);

	float depth = texture(depthMap, uv).r;
	if (depth >= 1.0) {
		reflectionColor = vec4(0.0);
		reflectionColor = vec4(0,0,0,1);
		return;
	}

	// Reconstruct view-space position
	vec4 posClip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 posView = invProjection * posClip;
	posView /= posView.w;

	vec3 normalView = normalize(texture(gNormal, uv).rgb);
	vec3 viewDir = normalize(posView.xyz);
	vec3 rayDirView = normalize(reflect(viewDir, normalView));

	// Reject rays going behind the camera
	if (rayDirView.z >= 0.0)
	{
		reflectionColor = vec4(0.0);
		return;
	}

	vec3 reflectedColor = TraceRay_ViewSpace(posView.xyz, rayDirView);
	reflectionColor = vec4(reflectedColor, 1.0);
	
	// DEBUG STUFF
	
	//trace the ray
	//vec3 outColor = TraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
	//reflectionColor = vec4(outColor, 1);
	
	//float outColor = DebugTraceRay(pixelPositionTexture, rayDirectionTexture, screenSpaceMaxDistance);
	//reflectionColor = vec4(pixelPositionTexture, 1);
	//reflectionColor = vec4(rayDirectionTexture, 1);
	//reflectionColor = vec4(screenSpaceMaxDistance, 0.0, 0.0, 1);
	//reflectionColor = vec4(reflectionView * 0.5 + 0.5, 1.0);
}
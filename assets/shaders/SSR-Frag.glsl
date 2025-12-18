#version 330 core

layout (location = 0) out vec4 reflectionColor;

in vec2 texCoords;

// GBuffer
uniform sampler2D gPosition;     // view-space position (for shading only)
uniform sampler2D gNormal;       // view-space normal
uniform sampler2D gColor;
uniform sampler2D gMetalRough;
uniform sampler2D gDepth;        // hardware depth buffer

// Camera
uniform mat4 projection;
uniform float uNear;
uniform float uFar;

// Screen
uniform vec2 screenSize;

// Constants
const int   MAX_STEPS  = 64;
const float MAX_DIST   = 50.0;
const float MIN_STEP   = 0.05;
const float THICKNESS  = 0.15;

// Linear depth reconstruction
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * uNear * uFar) /
           (uFar + uNear - z * (uFar - uNear));
}

// Stable per-pixel noise
vec2 Noise(vec2 uv)
{
    return fract(sin(vec2(
        dot(uv, vec2(12.9898, 78.233)),
        dot(uv, vec2(39.3468, 11.1351))
    )) * 43758.5453);
}

void main()
{
    // Material properties
    // ------------------------------------------------------------------
    vec2 mr = texture(gMetalRough, texCoords).rg;
    float metallic  = mr.r;
    float roughness = mr.g;

    if (metallic < 0.01)
        discard;

    // View-space inputs
    // ------------------------------------------------------------------
    vec3 viewPos    = texture(gPosition, texCoords).xyz;
    vec3 viewNormal = normalize(texture(gNormal, texCoords).xyz);

    if (length(viewPos) < 0.0001)
        discard;

    vec3 V = normalize(-viewPos);
    vec3 R = normalize(reflect(V, viewNormal));
	
	if (R.z >= -0.01) // Reject rays pointing behind camera
		discard;

    // Ray setup
    // ------------------------------------------------------------------
    float surfaceDepth = -viewPos.z;

    float camDist  = length(viewPos);
    float stepSize = max(MIN_STEP, camDist * 0.02);

    vec3 rayPos = viewPos + viewNormal * THICKNESS;

    // Stable jitter
    vec2 jitter = Noise(gl_FragCoord.xy / screenSize) * 2.0 - 1.0;
    R = normalize(R + vec3(jitter * 0.002, 0.0));

    vec2 hitUV = vec2(-1.0);
    bool hit = false;

    // Ray marching
    // ------------------------------------------------------------------
    for (int i = 0; i < MAX_STEPS; ++i)
    {
        rayPos += R * stepSize;

        if (length(rayPos - viewPos) > MAX_DIST)
            break;

        vec4 clip = projection * vec4(rayPos, 1.0);
        if (clip.w <= 0.0)
            break;

        vec3 ndc = clip.xyz / clip.w;
        vec2 uv  = ndc.xy * 0.5 + 0.5;

        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
            break;

        // Depth-buffer intersection test (THE FIX)
        // ------------------------------------------------------------------
        float depthSample = texture(gDepth, uv).r;
        float sceneDepth  = LinearizeDepth(depthSample);
        float rayDepth    = -rayPos.z;

        // Reject hits in front of the reflective surface
        if (sceneDepth <= surfaceDepth + THICKNESS)
            continue;

        // Detect ray crossing geometry
        if (rayDepth >= sceneDepth - THICKNESS)
        {
            hitUV = uv;
            hit = true;
            break;
        }
		
		//reflectionColor = vec4(vec3(rayDepth - sceneDepth), 1.0);
    }

    if (!hit)
        discard;

    // Shading
    // ------------------------------------------------------------------
    vec3 reflectedColor = texture(gColor, hitUV).rgb;
    vec3 baseColor      = texture(gColor, texCoords).rgb;

    vec3 F0 = mix(vec3(0.04), baseColor, metallic);
    float cosTheta = clamp(dot(viewNormal, V), 0.0, 1.0);
    vec3 fresnel = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);

    vec2 edge = abs(hitUV - 0.5) * 2.0;
    float edgeFade = clamp(1.0 - max(edge.x, edge.y), 0.0, 1.0);

    vec3 result = reflectedColor * fresnel * edgeFade * metallic;

    reflectionColor = vec4(result, 1.0);
	
	//float d = texture(gDepth, texCoords).r;
	//reflectionColor = vec4(vec3(d), 1.0);
	//return;
	
	/*
	float d = LinearizeDepth(texture(gDepth, texCoords).r) / uFar;
	reflectionColor = vec4(vec3(d), 1.0);
	return;
	*/
	
	//reflectionColor = vec4(metallic, 0.0, 0.0, 1.0);
	//reflectionColor = vec4(reflectedColor, 1.0);
	
	//reflectionColor = vec4(hitUV, 0.0, 1.0);
	
	//reflectionColor = vec4(viewNormal * 0.5 + 0.5, 1.0);
}

// Fragment shader (gBuffer.frag)
#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 fragPos;
in vec2 texCoord;
in vec3 normal;

// Material properties
uniform sampler2D texture_diffuse1;
uniform vec3 diffuseColor;

void main()
{    
    // Store the fragment position vector in the first gbuffer texture
    gPosition = fragPos;
    // Also store the per-fragment normals into the gbuffer
    gNormal = normalize(normal);
    // And the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(texture_diffuse1, texCoord).rgb * diffuseColor;
}
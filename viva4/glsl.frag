#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int textureIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler texSampler;
layout(binding = 2) uniform texture2D sampledImage[40];

void main() {
	outColor = texture(sampler2D(sampledImage[textureIndex],texSampler), fragTexCoord);		
	if(outColor.a == 0)
		discard;
}
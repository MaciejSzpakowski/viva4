#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in nonuniformEXT flat int textureIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler texSampler;
layout(binding = 2) uniform texture2D sampledImage[256];

void main() {
	if(textureIndex == 999999)
	{
		discard;
	}
	else if(textureIndex >= 0)
	{
		vec4 mask = vec4(fragColor,1);
		outColor = texture(sampler2D(sampledImage[textureIndex],texSampler), fragTexCoord);
		outColor.r *= mask.r;
		outColor.g *= mask.g;
		outColor.b *= mask.b;
	}
	else
	{
		outColor = vec4(fragColor,1);
	}
		
	if(outColor.a == 0)
		discard;
}
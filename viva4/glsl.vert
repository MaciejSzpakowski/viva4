#version 450
#extension GL_ARB_separate_shader_objects : enable
// https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)

struct transform
{
	float x;
	float y;
	float z;
	float sx;
	float sy;
	float rot;
	float ox;
	float oy;
	int textureInstance;
	float left;
	float top;
	float right;
	float bottom;
	float r;
	float g;
	float b;
    bool _fixed;
};

struct camera
{
	float aspectRatio;
	float x;
	float y;
	float rotation;
	float scale;
};

layout(binding = 0) uniform UniformBufferObject {
	camera cam;
    transform t[1000];
} ubo;

// first column is POS, second is UV
vec2 vertices[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2(-0.5, 0.5),
    vec2(0.5, -0.5),
	vec2(0.5, 0.5)
);

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat int textureIndex;

void main() {
	transform t = ubo.t[gl_InstanceIndex];
	
	vec2 uv[4] = vec2[](vec2(t.left, t.top),vec2(t.left, t.bottom),vec2(t.right, t.top),vec2(t.right, t.bottom));
	// camera
    // that is correct camera transformation
    // includes aspect ratio adjustment and it FIRST moves camera to postion and then scales
    // so when zooming, it always zooms around the center of the screen
	mat4 cam = mat4(
		1/ubo.cam.aspectRatio * ubo.cam.scale, 0, 0, 0,
		0, ubo.cam.scale, 0, 0,
		0, 0, 1, 0,
		1/ubo.cam.aspectRatio * ubo.cam.scale * -ubo.cam.x, ubo.cam.scale * -ubo.cam.y, 0, 1
	);
    // fixed means ignore camera except for aspect ratio
    if(t._fixed)
    {
        cam = mat4(
		1/ubo.cam.aspectRatio, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
    }
	// origin
	mat4 ori = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-t.ox, -t.oy, 0, 1
	);
	// scale
	mat4 sca = mat4(
		t.sx, 0, 0, 0,
		0, t.sy, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// rot
	mat4 rot = mat4(
		cos(t.rot), sin(t.rot), 0, 0,
		-sin(t.rot), cos(t.rot), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// loc
	mat4 loc = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		t.x, t.y, 0, 1 // z from transform is not put over here because it's on the vertex itself
	);
	vec4 v = vec4(vertices[gl_VertexIndex].x, vertices[gl_VertexIndex].y,  ubo.t[gl_InstanceIndex].z, 1.0);
	gl_Position = cam * loc * rot * sca * ori * v;
	fragColor = vec3(t.r,t.g,t.b);		
	fragTexCoord = uv[gl_VertexIndex];
	textureIndex = t.textureInstance;
}
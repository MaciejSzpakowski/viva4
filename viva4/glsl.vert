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
    transform t[10000];
} ubo;

// first column is POS, second is UV
vec2 vertices[8] = vec2[](
    vec2(-0.5, -0.5), vec2(0.0, 0.0),
    vec2(-0.5, 0.5), vec2(0.0, 1.0),
    vec2(0.5, -0.5), vec2(1.0, 0.0),
	vec2(0.5, 0.5), vec2(1.0, 1.0)
);

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out int textureIndex;

void main() {
	float width = 800;
	float height = 600;
	transform t = ubo.t[gl_InstanceIndex];
	// camera
	mat4 m1 = mat4(
		1/ubo.cam.aspectRatio * ubo.cam.scale, 0, 0, 0,
		0, ubo.cam.scale, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// origin
	mat4 m2 = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		t.ox, t.oy, 0, 1
	);
	// scale
	mat4 m3 = mat4(
		t.sx, 0, 0, 0,
		0, t.sy, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// rot
	mat4 m4 = mat4(
		cos(t.rot), sin(t.rot), 0, 0,
		-sin(t.rot), cos(t.rot), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
	// loc
	mat4 m5 = mat4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		t.x, t.y, 0, 1 // z from transform is not put over here because it's on the vertex itself
	);
	vec4 v = vec4(vertices[gl_VertexIndex * 2].x, vertices[gl_VertexIndex * 2].y,  ubo.t[gl_InstanceIndex].z, 1.0);
	gl_Position = m1 * m5 * m3 * m4 * m2 * v;
    fragColor = vec3(1.0,1.0,1.0);
	fragTexCoord = vec2(vertices[gl_VertexIndex * 2 + 1].x, vertices[gl_VertexIndex * 2 + 1].y);
	textureIndex = t.textureInstance;
}
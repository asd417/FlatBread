#version 450

#extension GL_KHR_vulkan_glsl : enable

//layout(location=0) in vec3 position;
//layout(location=1) in vec3 color;
//layout(location=2) in vec2 uv;

layout (location = 0) out vec2 fragOffset;


const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0,-1.0),
	vec2( 1.0,-1.0),
	vec2(-1.0, 1.0),
	vec2( 1.0,-1.0),
	vec2( 1.0, 1.0),
	vec2(-1.0, 1.0)
);

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionMatrix;
	mat4 viewMatrix;
} ubo;

//This is obviously inefficient because I can just use w of position in PointLight as a radius but for the sake of demonstrating pushcontant
layout(push_constant) uniform Push {
	mat3 transform;
	vec4 color;
} push;

void main(){
	fragOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRightWorld = {ubo.viewMatrix[0][0],ubo.viewMatrix[1][0],ubo.viewMatrix[2][0]};
	vec3 cameraUpWorld = {ubo.viewMatrix[0][1],ubo.viewMatrix[1][1],ubo.viewMatrix[2][1]};

	vec3 positions = fragOffset.x * cameraRightWorld + fragOffset.y * cameraUpWorld;
	positions[2] = 1;
	vec3 positionWorld = push.transform * positions;

	vec4 pos = ubo.projectionMatrix * ubo.viewMatrix * vec4(positionWorld,1.0);
	//pos.y = pos.y * -1;
	gl_Position = pos;

}
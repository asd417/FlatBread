#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionMatrix;
	mat4 viewMatrix;
} ubo;
layout (binding = 1) uniform sampler2D samplerColor;

const float M_PI = 3.1415926538;
const float M_E =  2.7182818284;

const float exponent = 4.4f;
const float offset = 365.0f;
//This is obviously inefficient because I can just use w of position in PointLight as a radius but for the sake of demonstrating pushcontant
layout(push_constant) uniform Push {
	mat3 transform;
	vec4 color;
} push;

float light_exponential(float dist){
	return pow(M_E,-exponent*dist/2)-exponent*0.00001*offset*dist/2;
}
float light_cos(float dist){
	return pow(cos(dist * M_PI / (2*2)),exponent);
}
void main(){
	float dist = sqrt(dot(fragOffset, fragOffset));
	if(dist > 2) {
		//Throw away this fragment and return
		discard;
	}

	//exponential
	//outColor = vec4(push.color.xyz, light_exponential(dist));

	//cosine
	outColor = vec4(vec3(0.5f,0.5f,0.5f)+light_cos(dist), light_cos(dist));
	//outColor = vec4(1.0f,1.0f,1.0f,1.0f);
}
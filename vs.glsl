#version 330 core

layout (location = 0) in vec3 vInPosition;
layout (location = 1) in vec3 vInNormal;
layout (location = 2) in vec2 vInTexCoord;

uniform MatrixBlock {
	mat4 mView;
	mat4 mWorld;
	mat4 mProjection;
};

out vec3 vOutWorldPos;
out vec3 vOutNormal;
out vec2 vOutTexCoord;

void main() {
	vec4 vWorldPos = mWorld*vec4(vInPosition, 1.0);
	gl_Position = mProjection*mView*vWorldPos;
	vOutWorldPos = vWorldPos.xyz;
	vOutTexCoord = vInTexCoord;
	vOutNormal = vInNormal;
}
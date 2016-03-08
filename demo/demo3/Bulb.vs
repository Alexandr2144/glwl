#version 330 core

layout (location = 0) in vec3 vInPosition;
layout (location = 1) in vec2 vInTexCoord;

uniform MatrixBlock {
	mat4 mView;
	mat4 mWorld;
	mat4 mProjection;
};

out vec3 vOutWorldPos;
out vec3 vOutViewPos;
out vec2 vOutTexCoord;

void main() {
	gl_Position = vec4(vInPosition, 1.0);

	vec4 View = vec4(0, 0, -1, 0);
	vOutViewPos = (mWorld*vec4(0, 0, -3, 0)).xyz;
	vOutWorldPos = (mWorld*(gl_Position+View)).xyz;
	vOutTexCoord = vInTexCoord;
}
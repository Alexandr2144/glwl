#version 330 core

layout (location = 0) in vec3 vPosition;

uniform Matrixes {
	mat4 mView;
	mat4 mWorld;
	mat4 mProjection;
};

out vec4 vWorldPos;

void main() {
	gl_Position = mProjection*mView*mWorld*vec4(vPosition, 1.0);

	vWorldPos = vec4(vPosition, 0.0f);
}
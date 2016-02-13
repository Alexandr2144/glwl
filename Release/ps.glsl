#version 330 core

uniform sampler2D texture;

struct PointLight {
	bool enable;
	vec4 color;
	vec3 position;
	vec3 attenuation;
};

struct Light {
	vec3 camera;
	vec4 ambient;

	int point_cnt;
	PointLight point[10];
};

struct Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;
	float roughness;
};

uniform MaterialBlock { 
	Material material; };
uniform LightBlock {
	Light light;

	bool ambient;
	bool diffuse;
	bool specular;
	bool phong;
};

in vec3 vOutWorldPos;
in vec3 vOutNormal;
in vec2 vOutTexCoord;

out vec4 vFragColor;

void main() {
	//if(!ambient) { vFragColor = material.emission * material.emission.w; return; }
	vFragColor = light.ambient * light.ambient.w * 
		material.ambient * material.ambient.w + material.emission * material.emission.w;
	//if(!diffuse) return;

	vec4 materialDiff = material.diffuse * material.diffuse.w;
	vec4 materialSpec = material.specular * material.specular.w;
	vec4 Color;

	vec3 L, R, H;
	vec3 N = vOutNormal;
	vec3 V = normalize(light.camera - vOutWorldPos);
	float NdotL, NdotH, NdotV, VdotH, attenuation;
	float NdotH2, NdotH2_r, m2;
	float G, F, D, K;

	for (int i = 0; i < light.point_cnt; i++) {
		if(!light.point[i].enable) continue;
		Color = light.point[i].color * light.point[i].color.w;

		L = light.point[i].position - vOutWorldPos;
		float attenuation = length(L); L /= attenuation;
		attenuation = 1/(
			light.point[i].attenuation.x + 
			attenuation*light.point[i].attenuation.y + 
			attenuation*attenuation*light.point[i].attenuation.z);
	
		NdotL = max(0, dot(L, N));
		vFragColor += Color * materialDiff * NdotL * attenuation;

		//R = reflect(-L, N);
		//vFragColor += Color * materialSpec * max(0, pow(dot(R, V), 16)) * attenuation;

		H = normalize(V + L);
		NdotH = max(dot(N, H), 1.0e-7);
		NdotV = max(dot(N, V), 0.0);
		VdotH = max(dot(V, H), 0.0);

		G = 2.0*NdotH / VdotH;
		G = min(1.0, G * min(NdotV, NdotL));

		m2 = material.roughness * material.roughness;
		NdotH2 = NdotH * NdotH;
		NdotH2_r = 1.0 / (NdotH2 * m2);
		D = exp((NdotH2 - 1.0) * ( NdotH2_r )) * NdotH2_r / (4.0 * NdotH2 );

		F = 1.0 / (1.0 + NdotV);

		K = min(1.0, (F*G*D)/(NdotV*NdotL + 1.0e-7));

		vFragColor += Color * materialSpec * K * attenuation;
	}
}
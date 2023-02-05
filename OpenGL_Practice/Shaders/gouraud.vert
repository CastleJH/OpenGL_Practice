#version 460
#define NUMLIGHT 5

uniform mat4 MVPMat;
uniform mat4 MVMat;
uniform mat3 MVMatInvTrans;  

struct LIGHT {
	vec4 pos; // assume point or direction in EC in this example shader
	vec4 ambient, diffuse, specular;
	vec4 distFactor; // compute this effect only if .w != 0.0f
	vec3 spotDir;
	float spotExpo;
	float spotAngle;
	bool isOn;
};

struct MATERIAL {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 emissive;
	float specExpo;
};

uniform vec4 glbAmbient;
uniform LIGHT lights[NUMLIGHT];
uniform MATERIAL material;

uniform float time;

uniform bool isCircle = false;

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
out vec4 vertClr;


vec4 lighting_equation(in vec3 V, in vec3 N) {
	vec4 returnClr;
	float distAtt, spotAtt, angleCos; 
	vec3 lightDir;

	returnClr = material.emissive + glbAmbient * material.ambient;
 
	for (int i = 0; i < NUMLIGHT; i++) {
		if (!lights[i].isOn) continue;

		distAtt = 1.0f;
		if (lights[i].pos.w != 0.0f) { // point light source
			lightDir = lights[i].pos.xyz - V.xyz;

			if (lights[i].distFactor.w  != 0.0f) {
				vec4 div;
				div.x = 1.0f;
				div.z = dot(lightDir, lightDir);
				div.y = sqrt(div.z);
				div.w = 0.0f;
				distAtt = 1.0f/dot(div, lights[i].distFactor);
			}

			lightDir = normalize(lightDir);

			if (lights[i].spotAngle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				float spotAngle = clamp(lights[i].spotAngle, 0.0f, 90.0f);
				vec3 spot_dir = normalize(lights[i].spotDir);

				angleCos = dot(-lightDir, spot_dir);
				if (angleCos >= cos(radians(spotAngle))) angleCos = pow(angleCos, lights[i].spotExpo);
				else angleCos = 0.0f;
				spotAtt = angleCos;

				if (i < 3 && isCircle) {
					if (angleCos > fract(time) && angleCos < fract(time + 0.1f)) spotAtt = 0;
				}
			}
		}
		else {  // directional light source
			lightDir = normalize(lights[i].pos.xyz);
		}	

		if (distAtt * spotAtt > 0.0f) {				
			vec4 localClr = lights[i].ambient * material.ambient;

			angleCos = dot(N, lightDir);
			if (angleCos > 0.0f) {
				localClr += lights[i].diffuse*material.diffuse*angleCos;
			
				vec3 H_EC = normalize(lightDir - normalize(V));
				angleCos = dot(N, H_EC); 
				if (angleCos > 0.0f) {
					localClr += lights[i].specular
										   *material.specular*pow(angleCos, material.specExpo);
				}
			}
			returnClr += distAtt * spotAtt * localClr;
		}
	}
 	return returnClr;

}

void main(void) {
	gl_Position = MVPMat*vec4(a_position, 1.0f);

	vertClr = lighting_equation(vec3(MVMat*vec4(a_position, 1.0f)), normalize(MVMatInvTrans*a_normal));
}
 
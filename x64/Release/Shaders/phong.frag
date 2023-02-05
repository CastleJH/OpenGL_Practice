#version 460
#define NUMLIGHT 5

struct LIGHT {
	vec4 pos;
	vec4 ambient, diffuse, specular;
	vec4 distFactor;
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

uniform sampler2D textureID;
uniform bool isTexture;

uniform bool isToon = false;
uniform bool isCircle = false;
uniform bool isCrack = false;
uniform vec3 wolfPos;
uniform float crackTime = -1.0f;

in vec3 vertPos;
in vec3 vertNorm;
in vec2 texCoord;

layout (location = 0) out vec4 finalClr;

vec4 lighting_equation(in vec3 V, in vec3 N, in vec4 clr) {
	vec4 returnClr;
	float distAtt, spotAtt, angleCos; 
	vec3 lightDir;

	returnClr = material.emissive + glbAmbient * material.ambient;

	for (int i = 0; i < NUMLIGHT; i++) {
		if (!lights[i].isOn) continue;

		distAtt = 1.0f;
		if (lights[i].pos.w != 0.0f) { 
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

			if (lights[i].spotAngle < 180.0f) { 
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
		else { 
			lightDir = normalize(lights[i].pos.xyz);
		}	

		if (distAtt * spotAtt > 0.0f) {				
			vec4 localClr = lights[i].ambient * material.ambient;

			angleCos = dot(N, lightDir);
			if (angleCos > 0.0f) {
				localClr += lights[i].diffuse*clr*angleCos;
			
				vec3 H_EC = normalize(lightDir - normalize(V));
				angleCos = dot(N, H_EC); 
				if (angleCos > 0.0f) {
					localClr += lights[i].specular * material.specular * pow(angleCos, material.specExpo);
				}
			}
			returnClr += distAtt * spotAtt * localClr;
		}
	}
 	return returnClr;
}

vec2 random(in vec2 point) {
	return -1.0f + 2.0f * fract(sin(vec2(dot(point,vec2(34.16f, 286.9f)), dot(point,vec2(175.7f,348.89f)))) * 24462.214987f);
}

float noise(in vec2 point) {
    vec2 flr = floor( point );
    vec2 frct = fract( point );

	vec2 source = frct * frct * (4.0f - 3.0f * frct);

    return mix(mix(dot(random(flr + vec2(0, 0)), frct - vec2(0, 0)), dot(random(flr + vec2(1, 0)), frct - vec2(1, 0)), source.x),
               mix(dot(random(flr + vec2(0, 1)), frct - vec2(0, 1)), dot(random(flr + vec2(1, 1)), frct - vec2(1, 1)), source.x), source.y);
}

void main(void) {
	vec4 clr;
	if (isTexture) clr = texture(textureID, texCoord);
	else clr = material.diffuse;
	finalClr = lighting_equation(vertPos, normalize(vertNorm), clr);
	if (isCrack && time < crackTime) {
		float darker = noise(vec2(vertPos));
		float distance = sqrt(dot(vertPos - wolfPos, vertPos - wolfPos));
		if (distance < 3.0f) finalClr = finalClr * (distance/(darker * 2));
	}
	if (isToon && material.diffuse.b >= 0.39f && material.diffuse.b <= 0.41f) {
		if (finalClr.r > 0.6f) finalClr.r = 0.9;
		else if (finalClr.r > 0.3f) finalClr.r = 0.5f;
		else finalClr.r = 0.05f;
		finalClr.g = finalClr.r;
		if (finalClr.b > 0.6f) finalClr.b = 0.8;
		else if (finalClr.b > 0.3f) finalClr.b = 0.4f;
		else finalClr.b = 0.0f;
	}
}
 
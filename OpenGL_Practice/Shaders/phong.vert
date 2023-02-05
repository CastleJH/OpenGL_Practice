#version 460

uniform mat4 MVPMat;
uniform mat4 MVMat;
uniform mat3 MVMatInvTrans;  

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;
out vec3 vertPos;
out vec3 vertNorm;
out vec2 texCoord;

void main(void) {	
	vertPos = vec3(MVMat*vec4(a_position, 1.0f));
	vertNorm = normalize(MVMatInvTrans*a_normal);  
	texCoord = a_tex_coord;

	gl_Position = MVPMat*vec4(a_position, 1.0f);
}


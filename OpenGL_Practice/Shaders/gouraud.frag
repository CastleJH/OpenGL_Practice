#version 460

in vec4 vertClr;

layout (location = 0) out vec4 finalClr;

void main(void) {	
	finalClr = vertClr;
}


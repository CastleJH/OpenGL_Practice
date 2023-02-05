#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <FreeImage/FreeImage.h>

#include "Shaders/LoadShaders.h"

#define NUMTEXTURE 2
#define TEXTUREROSE 0
#define TEXTUREFOOD 1
GLuint textures[NUMTEXTURE];

void TextureFromFile(char* filename) {
	FREE_IMAGE_FORMAT textureFormat;
	int bpp;
	FIBITMAP* pmap, * pmap32;

	int w, h;
	GLvoid* data;

	textureFormat = FreeImage_GetFileType(filename, 0);
	pmap = FreeImage_Load(textureFormat, filename);
	bpp = FreeImage_GetBPP(pmap);

	if (bpp == 32) pmap32 = pmap;
	else pmap32 = FreeImage_ConvertTo32Bits(pmap);

	w = FreeImage_GetWidth(pmap32);
	h = FreeImage_GetHeight(pmap32);
	data = FreeImage_GetBits(pmap32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	FreeImage_Unload(pmap32);
	if (bpp != 32)
		FreeImage_Unload(pmap);
}

void prepareTextures() {
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTUREROSE);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTUREROSE]);

	TextureFromFile((char*)"Data/rose.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTUREFOOD);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTUREFOOD]);

	TextureFromFile((char*)"Data/food.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/******************************* material and light *******************************/
typedef struct _light {
	bool isOn;
	float pos[4];
	float ambient[4], diffuse[4], specular[4];
	float spotDir[3];
	float spotAngle;
	float spotExpo;
	float distFactor[4];
} Light;

typedef struct _psLocLight {
	GLint isOn;
	GLint pos;
	GLint ambient, diffuse, specular;
	GLint spotDir;
	GLint spotAngle;
	GLint spotExpo;
	GLint distFactor;
} LocLight;

typedef struct _material {
	float ambient[4], diffuse[4], specular[4], emissive[4];
	float specExpo;
} Material;

typedef struct _psLocMaterial {
	GLint ambient, diffuse, specular, emissive;
	GLint specExpo;
} LocMaterial;

#define NUMMATERIAL 5
#define NUMLIGHT 5
Material materials[NUMMATERIAL];
Light lights[NUMLIGHT];

Material makeMaterial(
	float ar = 0.0f, float ag = 0.0f, float ab = 0.0f, float aa = 0.0f,
	float dr = 0.0f, float dg = 0.0f, float db = 0.0f, float da = 0.0f,
	float sr = 0.0f, float sg = 0.0f, float sb = 0.0f, float sa = 0.0f, float expo = 0.0f,
	float er = 0.0f, float eg = 0.0f, float eb = 0.0f, float ea = 0.0f) {
	Material material;
	material.ambient[0] = ar;
	material.ambient[1] = ag;
	material.ambient[2] = ab;
	material.ambient[3] = aa;

	material.diffuse[0] = dr;
	material.diffuse[1] = dg;
	material.diffuse[2] = db;
	material.diffuse[3] = da;

	material.specular[0] = sr;
	material.specular[1] = sg;
	material.specular[2] = sb;
	material.specular[3] = sa;

	material.specExpo = expo;

	material.emissive[0] = er;
	material.emissive[1] = eg;
	material.emissive[2] = eb;
	material.emissive[3] = ea;

	return material;
}

Light makeLight(
	float ar = 0.0f, float ag = 0.0f, float ab = 0.0f, float aa = 0.0f,
	float dr = 0.0f, float dg = 0.0f, float db = 0.0f, float da = 0.0f,
	float sr = 0.0f, float sg = 0.0f, float sb = 0.0f, float sa = 0.0f) {
	Light light;

	light.isOn = true;
	light.pos[0] = 0.0f;
	light.pos[1] = 0.0f;
	light.pos[2] = 0.0f;
	light.pos[3] = 1.0f;

	light.ambient[0] = ar;
	light.ambient[1] = ag;
	light.ambient[2] = ab;
	light.ambient[3] = aa;

	light.diffuse[0] = dr;
	light.diffuse[1] = dg;
	light.diffuse[2] = db;
	light.diffuse[3] = da;

	light.specular[0] = sr;
	light.specular[1] = sg;
	light.specular[2] = sb;
	light.specular[3] = sa;

	light.pos[0] = 0.0f;
	light.pos[1] = 0.0f;
	light.pos[2] = 0.0f;
	light.pos[3] = 1.0f;

	light.spotDir[0] = 0.0f;
	light.spotDir[1] = -1.0f;
	light.spotDir[2] = 0.0f;

	light.spotExpo = 27;

	light.spotAngle = 180.0f;

	light.distFactor[0] = 1.0f;
	light.distFactor[1] = 0.0f;
	light.distFactor[2] = 0.0f;
	light.distFactor[3] = 0.0f;

	return light;
}

void initializeMaterialsAndLights() {
	materials[0] = makeMaterial
	(0.05f, 0.05f, 0.05f, 1,
		0.2f, 0.2f, 0.2f, 1,
		0.4f, 0.4f, 0.4f, 1, 37,
		0, 0, 0, 1);
	materials[1] = makeMaterial
	(0.05f, 0.05f, 0, 1,
		0.5f, 0.5f, 0.4f, 1,
		0.7f, 0.7f, 0.04f, 1, 59,
		0, 0, 0, 1);
	materials[2] = makeMaterial
	(0.05f, 0, 0.05f, 1,
		0.5f, 0.4f, 0.5f, 1,
		0.7f, 0.04f, 0.7f, 1, 61,
		0, 0, 0, 1);
	materials[3] = makeMaterial
	(0.05f, 0.05f, 0.05f, 1,
		0.2f, 0.2f, 0.2f, 1,
		0.4f, 0.4f, 0.4f, 1, 37,
		0, 0, 0, 1);
	materials[4] = makeMaterial
	(0.05f, 0.05f, 0.05f, 1,
		0.8f, 0.8f, 0.8f, 1,
		0.04f, 0.04f, 0.04f, 1, 2.5f,
		0, 0, 0, 1);

	lights[0] = makeLight //R in WC
	(0.3f, 0.3f, 0.3f, 1,
		1.0f, 0.25f, 0.25f, 1,
		1.0f, 0.25f, 0.25f, 1);

	lights[1] = makeLight //G in WC
	(0.3f, 0.3f, 0.3f, 1,
		0.25f, 1.0f, 0.25f, 1,
		0.25f, 1.0f, 0.25f, 1);

	lights[2] = makeLight //B in WC
	(0.3f, 0.3f, 0.3f, 1,
		0.25f, 0.25f, 1.0f, 1,
		0.25f, 0.25f, 1.0f, 1);

	lights[3] = makeLight //W in EC
	(0.2f, 0.2f, 0.2f, 1,
		0.9f, 0.9f, 0.9f, 1,
		0.8f, 0.8f, 0.8f, 1);

	lights[4] = makeLight //W in MC
	(0.2f, 0.2f, 0.2f, 1,
		0.4f, 0.4f, 0.4f, 1,
		0.3f, 0.3f, 0.3f, 1);

	lights[0].isOn = true;
	lights[0].pos[0] = 2; lights[0].pos[1] = 6; lights[0].pos[2] = 2;
	lights[0].spotDir[0] = 0; lights[0].spotDir[1] = -1.0f; lights[0].spotDir[2] = 0;
	lights[0].spotAngle = 45;
	lights[0].spotExpo = 20;

	lights[1].isOn = true;
	lights[1].pos[0] = -2; lights[1].pos[1] = 6; lights[1].pos[2] = 2;
	lights[1].spotDir[0] = 0; lights[1].spotDir[1] = -1.0f; lights[1].spotDir[2] = 0;
	lights[1].spotAngle = 45;
	lights[1].spotExpo = 20;

	lights[2].isOn = true;
	lights[2].pos[0] = 0; lights[2].pos[1] = 6; lights[2].pos[2] = -2;
	lights[2].spotDir[0] = 0; lights[2].spotDir[2] = -1.0f; lights[2].spotDir[2] = 0;
	lights[2].spotAngle = 45;
	lights[2].spotExpo = 20;

	lights[3].isOn = true;
	lights[3].spotAngle = 45;
	lights[3].spotExpo = 50.0f;

	lights[4].isOn = true;
	lights[4].spotAngle = 15.0f;
	lights[4].spotExpo = 3.0f;
}

/******************************* Shader *******************************/
GLuint gouraudShader;
GLint grLocGlbAmbient;
GLint grLocMVPMat, grLocMVMat, grLocMVMatInvTrans;
LocMaterial grLocMaterial;
LocLight grLocLights[NUMLIGHT];
GLint grLocTime;
GLint grLocIsCircle;

GLuint phongShader;
GLint psLocGlbAmbient;
GLint psLocMVPMat, psLocMVMat, psLocMVMatInvTrans;
LocMaterial psLocMaterial;
GLint psLocTexture, psLocIsTexture;
LocLight psLocLights[NUMLIGHT];
GLint psLocTime, psLocCrackTime;
GLint psLocIsToon, psLocIsCircle, psLocIsCrack, psLocWolfPos;

void prepareShaderProgram() {
	ShaderInfo gouraudShaderGen[3] = {
		{ GL_VERTEX_SHADER, "Shaders/gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/gouraud.frag" },
		{ GL_NONE, NULL }
	};
	gouraudShader = LoadShaders(gouraudShaderGen);
	grLocMVPMat = glGetUniformLocation(gouraudShader, "MVPMat");
	grLocMVMat = glGetUniformLocation(gouraudShader, "MVMat");
	grLocMVMatInvTrans = glGetUniformLocation(gouraudShader, "MVMatInvTrans");
	grLocGlbAmbient = glGetUniformLocation(gouraudShader, "glbAmbient");

	char string[256];
	for (int i = 0; i < NUMLIGHT; i++) {
		sprintf(string, "lights[%d].isOn", i);
		grLocLights[i].isOn = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].pos", i);
		grLocLights[i].pos = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].ambient", i);
		grLocLights[i].ambient = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].diffuse", i);
		grLocLights[i].diffuse = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].specular", i);
		grLocLights[i].specular = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].spotDir", i);
		grLocLights[i].spotDir = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].spotExpo", i);
		grLocLights[i].spotExpo = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].spotAngle", i);
		grLocLights[i].spotAngle = glGetUniformLocation(gouraudShader, string);
		sprintf(string, "lights[%d].distFactor", i);
		grLocLights[i].distFactor = glGetUniformLocation(gouraudShader, string);
	}

	grLocMaterial.ambient = glGetUniformLocation(gouraudShader, "material.ambient");
	grLocMaterial.diffuse = glGetUniformLocation(gouraudShader, "material.diffuse");
	grLocMaterial.specular = glGetUniformLocation(gouraudShader, "material.specular");
	grLocMaterial.emissive = glGetUniformLocation(gouraudShader, "material.emissive");
	grLocMaterial.specExpo = glGetUniformLocation(gouraudShader, "material.specExpo");
	
	grLocTime = glGetUniformLocation(gouraudShader, "time");
	grLocIsCircle = glGetUniformLocation(gouraudShader, "isCircle");

	ShaderInfo phongShaderGen[3] = {
		{ GL_VERTEX_SHADER, "Shaders/phong.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/phong.frag" },
		{ GL_NONE, NULL }
	};
	phongShader = LoadShaders(phongShaderGen);
	psLocMVPMat = glGetUniformLocation(phongShader, "MVPMat");
	psLocMVMat = glGetUniformLocation(phongShader, "MVMat");
	psLocMVMatInvTrans = glGetUniformLocation(phongShader, "MVMatInvTrans");
	psLocGlbAmbient = glGetUniformLocation(phongShader, "glbAmbient");

	for (int i = 0; i < NUMLIGHT; i++) {
		sprintf(string, "lights[%d].isOn", i);
		psLocLights[i].isOn = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].pos", i);
		psLocLights[i].pos = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].ambient", i);
		psLocLights[i].ambient = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].diffuse", i);
		psLocLights[i].diffuse = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].specular", i);
		psLocLights[i].specular = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].spotDir", i);
		psLocLights[i].spotDir = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].spotExpo", i);
		psLocLights[i].spotExpo = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].spotAngle", i);
		psLocLights[i].spotAngle = glGetUniformLocation(phongShader, string);
		sprintf(string, "lights[%d].distFactor", i);
		psLocLights[i].distFactor = glGetUniformLocation(phongShader, string);
	}

	psLocMaterial.ambient = glGetUniformLocation(phongShader, "material.ambient");
	psLocMaterial.diffuse = glGetUniformLocation(phongShader, "material.diffuse");
	psLocMaterial.specular = glGetUniformLocation(phongShader, "material.specular");
	psLocMaterial.emissive = glGetUniformLocation(phongShader, "material.emissive");
	psLocMaterial.specExpo = glGetUniformLocation(phongShader, "material.specExpo");

	psLocTime = glGetUniformLocation(phongShader, "time");

	psLocTexture = glGetUniformLocation(phongShader, "textureID");
	psLocIsTexture = glGetUniformLocation(phongShader, "isTexture");
	psLocIsToon = glGetUniformLocation(phongShader, "isToon");
	psLocIsCircle = glGetUniformLocation(phongShader, "isCircle");
	psLocIsCrack = glGetUniformLocation(phongShader, "isCrack");
	psLocWolfPos = glGetUniformLocation(phongShader, "wolfPos");
	psLocCrackTime = glGetUniformLocation(phongShader, "crackTime");
}

void passMaterialToShader(int ID, bool isPhong) {
	if (isPhong) {
		glUniform4fv(psLocMaterial.ambient, 1, materials[ID].ambient);
		glUniform4fv(psLocMaterial.diffuse, 1, materials[ID].diffuse);
		glUniform4fv(psLocMaterial.specular, 1, materials[ID].specular);
		glUniform1f(psLocMaterial.specExpo, materials[ID].specExpo);
		glUniform4fv(psLocMaterial.emissive, 1, materials[ID].emissive);
	}
	else {
		glUniform4fv(grLocMaterial.ambient, 1, materials[ID].ambient);
		glUniform4fv(grLocMaterial.diffuse, 1, materials[ID].diffuse);
		glUniform4fv(grLocMaterial.specular, 1, materials[ID].specular);
		glUniform1f(grLocMaterial.specExpo, materials[ID].specExpo);
		glUniform4fv(grLocMaterial.emissive, 1, materials[ID].emissive);
	}
}

void passLightOnOffToShader(int ID, bool isPhong) {
	if (isPhong) glUniform1i(psLocLights[ID].isOn, lights[ID].isOn);
	else glUniform1i(grLocLights[ID].isOn, lights[ID].isOn);
}

/******************************* floor *******************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

GLuint floor_VBO, floor_VAO;
GLfloat floor_vertices[12][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
#define FLOOR_MAT_ID 4

void prepareFloor(void) {
	glGenBuffers(1, &floor_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), &floor_vertices[0][0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &floor_VAO);
	glBindVertexArray(floor_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


/******************************* loading models *******************************/
#define NUMMODELS 4
#define ID_BEN 0
#define ID_SPIDER 1
#define ID_WOLF 2
#define ID_DRAGON 3

int frame_num[NUMMODELS];
int frame_timer[NUMMODELS];
int* model_n_triangles[NUMMODELS];
int* model_vertex_offset[NUMMODELS];
GLfloat** model_vertices[NUMMODELS];
GLuint model_VBO[NUMMODELS], model_VAO[NUMMODELS];

int readModel(GLfloat** object, char* filename, int bytes_per_primitive) {
	int n_triangles;
	FILE* fp;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "%s: [read fail]\n", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "%s: [memory fail]\n", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	fclose(fp);

	return n_triangles;
}

void getFilename(int id, int i, char* filename) {
	switch (id) {
	case ID_BEN:
		sprintf(filename, "Data/Dynamic/ben/ben_vn%d%d.geom", i / 10, i % 10);
		break;
	case ID_SPIDER:
		sprintf(filename, "Data/Dynamic/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		break;
	case ID_WOLF:
		sprintf(filename, "Data/Dynamic/wolf/wolf_%d%d_vnt.geom", i / 10, i % 10);
		break;
	case ID_DRAGON:
		sprintf(filename, "Data/Static/dragon_vnt.geom");
		break;
	}
}

void initializeModelsSpace() {
	frame_num[ID_BEN] = 30;
	frame_num[ID_SPIDER] = 16;
	frame_num[ID_WOLF] = 17;
	frame_num[ID_DRAGON] = 1;
	int i;
	for (i = 0; i < NUMMODELS; i++) {
		frame_timer[i] = 0;
		model_n_triangles[i] = new int[frame_num[i]];
		model_vertex_offset[i] = new int[frame_num[i]];
		model_vertices[i] = new GLfloat * [frame_num[i]];
	}
}

void prepareModels(int id) {
	int i, totalTriangle = 0;
	char filename[512];

	int bpv = 8 * sizeof(float);
	int bpt = 3 * bpv;

	for (i = 0; i < frame_num[id]; i++) {
		getFilename(id, i, filename);
		model_n_triangles[id][i] = readModel(&model_vertices[id][i], filename, bpt);
		totalTriangle += model_n_triangles[id][i];
		if (i == 0) model_vertex_offset[id][i] = 0;
		else model_vertex_offset[id][i] =
			model_vertex_offset[id][i - 1] + 3 * model_n_triangles[id][i - 1];
	}

	glGenBuffers(1, &model_VBO[id]);

	glBindBuffer(GL_ARRAY_BUFFER, model_VBO[id]);
	glBufferData(GL_ARRAY_BUFFER, totalTriangle * bpt, NULL, GL_STATIC_DRAW);

	for (i = 0; i < frame_num[id]; i++)
		glBufferSubData(GL_ARRAY_BUFFER, model_vertex_offset[id][i] * bpv,
			model_n_triangles[id][i] * bpt, model_vertices[id][i]);

	for (i = 0; i < frame_num[id]; i++) free(model_vertices[id][i]);
	free(model_vertices[id]);

	glGenVertexArrays(1, &model_VAO[id]);
	glBindVertexArray(model_VAO[id]);

	glBindBuffer(GL_ARRAY_BUFFER, model_VBO[id]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, bpv, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, bpv, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, bpv, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

/******************************* drawing *******************************/
glm::mat4 MVPMat, MVMat;
glm::mat3 MVMatInvTrans;
glm::mat4 VMat, PMat;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define psLoc_POSITION 0
#define psLoc_NORMAL 1

bool isMove[NUMMODELS] = { true, true, true };

void putLightEC() {
	glUseProgram(phongShader);
	glm::vec4 lightPos;
	glm::vec3 spotDir;
	lightPos = glm::vec4(0, 0, 0, 1);
	spotDir = glm::vec3(0, 0, -1);
	glUniform4fv(psLocLights[3].pos, 1, &lightPos[0]);
	glUniform3fv(psLocLights[3].spotDir, 1, &spotDir[0]);
	glUseProgram(0);

	glUseProgram(gouraudShader);
	glUniform4fv(grLocLights[3].pos, 1, &lightPos[0]);
	glUniform3fv(grLocLights[3].spotDir, 1, &spotDir[0]);
	glUseProgram(0);
}

void putLightWC() {
	glUseProgram(phongShader);
	glm::vec4 lightPos;
	glm::vec3 spotDir;
	for (int i = 0; i < 3; i++) {
		lightPos = VMat * glm::vec4(lights[i].pos[0], lights[i].pos[1], lights[i].pos[2], lights[i].pos[3]);
		spotDir = glm::mat3(VMat) * glm::vec3(lights[i].spotDir[0], lights[i].spotDir[1], lights[i].spotDir[2]);
		glUniform4fv(psLocLights[i].pos, 1, &lightPos[0]);
		glUniform3fv(psLocLights[i].spotDir, 1, &spotDir[0]);
	}
	glUseProgram(0);

	glUseProgram(gouraudShader);
	for (int i = 0; i < 3; i++) {
		lightPos = VMat * glm::vec4(lights[i].pos[0], lights[i].pos[1], lights[i].pos[2], lights[i].pos[3]);
		spotDir = glm::mat3(VMat) * glm::vec3(lights[i].spotDir[0], lights[i].spotDir[1], lights[i].spotDir[2]);
		glUniform4fv(grLocLights[i].pos, 1, &lightPos[0]);
		glUniform3fv(grLocLights[i].spotDir, 1, &spotDir[0]);
	}
	glUseProgram(0);
}


//bool sendCrackInfo = false;
glm::vec3 wolfPos;
glm::mat4 WMat, WVMat;
void putLightMC(GLuint lastShader) {
	glUseProgram(0);
	glUseProgram(phongShader);
	glm::vec4 lightPos;
	glm::vec3 spotDir;
	lightPos = MVMat * glm::vec4(0, 7, 0, 1);
	spotDir = glm::mat3(VMat) * glm::vec3(0, -1, 0);
	glUniform4fv(psLocLights[4].pos, 1, &lightPos[0]);
	glUniform3fv(psLocLights[4].spotDir, 1, &spotDir[0]);
	WVMat = VMat * WMat;
	wolfPos = glm::vec3(WVMat * glm::vec4(0, 0, 0, 1));
	glUniform3fv(psLocWolfPos, 1, &wolfPos[0]);
	glUseProgram(0);

	glUseProgram(gouraudShader);
	glUniform4fv(grLocLights[4].pos, 1, &lightPos[0]);
	glUniform3fv(grLocLights[4].spotDir, 1, &spotDir[0]);
	glUseProgram(0);

	glUseProgram(lastShader);
}

void drawModel(int id, bool isPhong) {
	passMaterialToShader(id, isPhong);
	glBindVertexArray(model_VAO[id]);
	glDrawArrays(GL_TRIANGLES, model_vertex_offset[id][frame_timer[id]], 3 * model_n_triangles[id][frame_timer[id]]);
	glBindVertexArray(0);
}

//Phong Shader
void drawFloor(void) {
	glUseProgram(phongShader);

	glUniform1i(psLocIsTexture, false);

	MVMat = glm::translate(VMat, glm::vec3(-7.0f, 0.0f, 7.0f));
	MVMat = glm::scale(MVMat, glm::vec3(14.0f, 1.0f, 14.0f));
	MVMat = glm::rotate(MVMat, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::mat3(glm::inverseTranspose(MVMat));

	glUniformMatrix4fv(psLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(psLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(psLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);

	passMaterialToShader(FLOOR_MAT_ID, true);
	glBindVertexArray(floor_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	MVMat = glm::translate(VMat, glm::vec3(-7.0f, 0.0f, -5.0f));
	MVMat = glm::scale(MVMat, glm::vec3(14.0f, 14.0f, 1.0f));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::inverseTranspose(glm::mat3(MVMat));

	glUniformMatrix4fv(psLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(psLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(psLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);

	glBindVertexArray(floor_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glUseProgram(0);
}

//Phong Shader
int ben_angle;
void drawBen() {
	glUseProgram(phongShader);

	glUniform1i(psLocIsTexture, true);
	glUniform1i(psLocTexture, TEXTUREFOOD);

	if (isMove[ID_BEN]) ben_angle++; ben_angle %= 360;
	MVMat = glm::rotate(VMat, ben_angle * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	MVMat = glm::translate(MVMat, glm::vec3(2.0f, 0.0f, 0.0f));
	MVMat = glm::scale(MVMat, glm::vec3(2.0f, -2.0f, 2.0f));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::mat3(glm::inverseTranspose(MVMat));
	glUniformMatrix4fv(psLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(psLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(psLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);
	drawModel(ID_BEN, true);

	glUseProgram(0);
}

//Phong Shader
int spider_timer;
float spider_size = 0.8f, spider_angle = 0.0f;
float spiderX = 3.0f, spiderY = 0.0f, spiderZ = 3.0f;
void drawSpider(bool showAxes = false) {
	glUseProgram(phongShader);

	glUniform1i(psLocIsTexture, false);

	if (isMove[ID_SPIDER]) {
		spider_timer++; spider_timer %= 96;
		if (spider_timer < 10) spider_size -= 0.03f;	//축소
		else if (spider_timer < 30) spiderY += 0.4f;	//상승
		else if (spider_timer < 35) spiderY += 0;	//대기
		else if (spider_timer < 36) {	//새 위치
			spiderX = (rand() % 9) - 4;
			spiderZ = (rand() % 9) - 4;
		}
		else if (spider_timer < 56) spiderY -= 0.4f;	//하강
		else if (spider_timer < 66) spider_size += 0.03f;	//확대
		else if (spider_timer < 96) { spider_angle += 12; ben_angle %= 360; } //회전
	}
	MVMat = glm::translate(VMat, glm::vec3(spiderX, spiderY, spiderZ));
	MVMat = glm::rotate(MVMat, spider_angle * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	MVMat = glm::scale(MVMat, glm::vec3(spider_size, -spider_size, spider_size));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::mat3(glm::inverseTranspose(MVMat));
	glUniformMatrix4fv(psLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(psLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(psLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);
	drawModel(ID_SPIDER, true);

	glUseProgram(0);
}

//Gouraud Shader
int wolfDir = 3, wolfAngle = 180;
float wolfX = -3.0f, wolfY = 0.0f, wolfZ = 3.0f;
float wolfXZSpeed = 0.1f, wolfYSpeed = 0.5f;
float gravity = 0.03f;
bool isJumping = true;
float time = 0;
bool sendCrackInfo = false;
void drawWolf(bool showAxes = false) {
	glUseProgram(phongShader);

	if (isMove[ID_WOLF]) {
		switch (wolfDir) {
		case 0:
			wolfX += wolfXZSpeed;
			if (wolfX >= 3.0f) {
				wolfDir++; wolfAngle = 0; isJumping = true;
			}
			break;
		case 1:
			wolfZ += wolfXZSpeed;
			if (wolfZ >= 3.0f) {
				wolfDir++; wolfAngle = -90; isJumping = true;
			}
			break;
		case 2:
			wolfX -= wolfXZSpeed;
			if (wolfX <= -3.0f) {
				wolfDir++; wolfAngle = 180; isJumping = true;
			}
			break;
		case 3:
			wolfZ -= wolfXZSpeed;
			if (wolfZ <= -3.0f) {
				wolfDir = 0; wolfAngle = 90; isJumping = true;
			}
			break;
		}
		if (isJumping) {
			wolfYSpeed -= gravity;
			wolfY += wolfYSpeed;
			if (wolfY <= 0) { 
				wolfY = 0; wolfYSpeed = 0.5f; isJumping = false; 
				glUniform1f(psLocCrackTime, time + 0.5f);
				sendCrackInfo = true;
			}
		}
	}
	MVMat = glm::translate(VMat, glm::vec3(wolfX, wolfY, wolfZ));
	MVMat = glm::rotate(MVMat, wolfAngle * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	if (sendCrackInfo) {
		sendCrackInfo = false;
		WMat = glm::translate(glm::mat4(1.0f), glm::vec3(wolfX, wolfY, wolfZ));
		WMat = glm::rotate(WMat, wolfAngle * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	putLightMC(gouraudShader);
	MVMat = glm::scale(MVMat, glm::vec3(2.0f, 2.0f, 2.0f));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::mat3(glm::inverseTranspose(MVMat));
	glUniformMatrix4fv(grLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(grLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(grLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);
	drawModel(ID_WOLF, false);

	glUseProgram(0);
}

//gouraud Shader
void drawDragon() {
	glUseProgram(phongShader);

	glUniform1i(psLocIsTexture, true);
	glUniform1i(psLocTexture, TEXTUREROSE);

	MVMat = glm::translate(VMat, glm::vec3(0.5f, 1, 0.f));
	MVMat = glm::rotate(MVMat, -90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	MVMat = glm::scale(MVMat, glm::vec3(0.1f, 0.1f, 0.1f));
	MVPMat = PMat * MVMat;
	MVMatInvTrans = glm::mat3(glm::inverseTranspose(MVMat));
	glUniformMatrix4fv(psLocMVPMat, 1, GL_FALSE, &MVPMat[0][0]);
	glUniformMatrix4fv(psLocMVMat, 1, GL_FALSE, &MVMat[0][0]);
	glUniformMatrix3fv(psLocMVMatInvTrans, 1, GL_FALSE, &MVMatInvTrans[0][0]);
	drawModel(ID_DRAGON, true);

	glUseProgram(0);
}

/******************************* Camera Control *******************************/
float zoomCam = 30.0f;
int mouseX = 0, mouseY = 0;
float aspect_ratio;
glm::vec3 camPos = { 0.0f, 3.0f, 10.0f };
glm::vec3 camLook = { 0.0f, 1.0f, 0.0f };
glm::vec3 camUp = { 0.0f, 1.0f, 0.0f };
bool isTiltPressed = false;
void controlCam(int axes, bool isTilt, float velo) {
	if (isTiltPressed) {
		int winWidth = glutGet(GLUT_WINDOW_WIDTH);
		int winHeight = glutGet(GLUT_WINDOW_HEIGHT);
		int min = winWidth + winHeight;
		int newDist;
		int axes = 0;
		float velo = 0;
		if ((newDist = glm::distance(glm::vec2(mouseX, mouseY), glm::vec2(0, winHeight / 2))) < min) {
			min = newDist;
			axes = 1; velo = -1.0f;
		}
		if ((newDist = glm::distance(glm::vec2(mouseX, mouseY), glm::vec2(winWidth, winHeight / 2))) < min) {
			min = newDist;
			axes = 1; velo = 1.0f;
		}
		if ((newDist = glm::distance(glm::vec2(mouseX, mouseY), glm::vec2(winWidth / 2, 0))) < min) {
			min = newDist;
			axes = 0; velo = 1.0f;
		}
		if ((newDist = glm::distance(glm::vec2(mouseX, mouseY), glm::vec2(winWidth / 2, winHeight / 2))) < min) {
			min = newDist;
			axes = 0; velo = -1.0f;
		}

		glm::vec3 front = glm::normalize(camLook - camPos);
		if (axes == 0) { //u
			glm::vec3 right = glm::cross(front, camUp);
			glm::vec3 normalUp = -1.0f * glm::normalize(glm::cross(front, right));
			glm::vec3 lookUp = normalUp * tan(velo * TO_RADIAN);
			camLook = camPos + front + lookUp;
			camUp = normalUp;
		}
		else { //v
			glm::vec3 normalRight = glm::normalize(glm::cross(front, camUp));
			glm::vec3 lookRight = normalRight * tan(velo * TO_RADIAN);
			camLook = camPos + front + lookRight;
		}
	}
	else if (!isTilt){
		glm::vec3 front = camLook - camPos;
		if (axes == 0) { //u
			glm::vec3 right = glm::normalize(glm::cross(front, camUp));
			camPos += (right * velo);
			camLook += (right * velo);
		}
		else if (axes == 1) { //v
			glm::vec3 right = glm::cross(front, camUp);
			glm::vec3 realUp = -1.0f * glm::normalize(glm::cross(front, right));
			camPos += (realUp * velo);
			camLook += (realUp * velo);
		}
		else if (axes == 2) { //-n
			camLook += (glm::normalize(front) * velo);
			camPos += (glm::normalize(front) * velo);
		}
	}
	VMat = glm::lookAt(camPos, camLook, camUp);
	putLightWC();
}

void controlPerspective(bool isZoom) {
	if (isZoom) {
		zoomCam *= 0.95;
		if (zoomCam < 5) zoomCam = 5;;
	}
	else {
		zoomCam *= 1.05;
		if (zoomCam > 70) zoomCam = 70;
	}
	PMat = glm::perspective(zoomCam * TO_RADIAN, aspect_ratio, 0.1f, 1000.0f);
}



/******************************* Display *******************************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	controlCam(0, true, 0);

	time += 0.01f;
	glUseProgram(phongShader);
	glUniform1f(psLocTime, time);
	glUseProgram(0);
	glUseProgram(gouraudShader);
	glUniform1f(grLocTime, time);
	glUseProgram(0);

	drawWolf();
	drawBen();
	drawSpider();
	drawFloor();
	drawDragon();

	glutSwapBuffers();
}

bool isToon = false;
bool isCircle = false;
bool isCrack = false;
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		isMove[0] = !isMove[0];
		break;
	case '2':
		isMove[1] = !isMove[1];
		break;
	case '3':
		isMove[2] = !isMove[2];
		break;
	case '8':
		isToon = !isToon;
		glUseProgram(phongShader);
		glUniform1i(psLocIsToon, isToon);
		glUseProgram(0);
		break;
	case '9':
		isCircle = !isCircle;
		glUseProgram(gouraudShader);
		glUniform1i(grLocIsCircle, isCircle);
		glUseProgram(0);
		glUseProgram(phongShader);
		glUniform1i(psLocIsCircle, isCircle);
		glUseProgram(0);
		break;
	case '0':
		isCrack = !isCrack;
		glUseProgram(phongShader);
		glUniform1i(psLocIsCrack, isCrack);
		glUseProgram(0);
		break;
	case 'z':
		controlCam(0, false, -0.1f);
		break;
	case 'c':
		controlCam(0, false, 0.1f);
		break;
	case 'x':
		controlCam(1, false, -0.1f);
		break;
	case 's':
		controlCam(1, false, 0.1f);
		break;
	case 'v':
		controlCam(2, false, -0.2f);
		break;
	case 'f':
		controlCam(2, false, 0.2f);
		break;
	case 'i':
		controlPerspective(true);
		break;
	case 'o':
		controlPerspective(false);
		break;
	case 'w':
		lights[0].isOn = !lights[0].isOn;
		lights[1].isOn = lights[0].isOn;
		lights[2].isOn = lights[0].isOn;
		glUseProgram(phongShader);
		passLightOnOffToShader(0, true);
		passLightOnOffToShader(1, true);
		passLightOnOffToShader(2, true);
		glUseProgram(0);

		glUseProgram(gouraudShader);
		passLightOnOffToShader(0, false);
		passLightOnOffToShader(1, false);
		passLightOnOffToShader(2, false);
		glUseProgram(0);
		break;
	case 'm':
		lights[4].isOn = !lights[4].isOn;
		glUseProgram(phongShader);
		passLightOnOffToShader(4, true);
		glUseProgram(0);

		glUseProgram(gouraudShader);
		passLightOnOffToShader(4, false);
		glUseProgram(0);
		break;
	case 'e':
		lights[3].isOn = !lights[3].isOn;
		glUseProgram(phongShader);
		passLightOnOffToShader(3, true);
		glUseProgram(0);

		glUseProgram(gouraudShader);
		passLightOnOffToShader(3, false);
		glUseProgram(0);
		break;
	case 27: // ESC key
		glutLeaveMainLoop();
		break;
	}
}

void mousepress(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isTiltPressed = true;
		mouseX = x; mouseY = y;
		controlCam(0, true, 0);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		isTiltPressed = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		camPos = { 0.0f, 3.0f, 10.0f };
		camLook = { 0.0f, 1.0f, 0.0f };
		camUp = { 0.0f, 1.0f, 0.0f };
		zoomCam = 30.0f;
		VMat = glm::lookAt(camPos, camLook, camUp);
		PMat = glm::perspective(zoomCam * TO_RADIAN, aspect_ratio, 0.1f, 1000.f);
	}
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);

	aspect_ratio = (float)width / height;
	PMat = glm::perspective(zoomCam * TO_RADIAN, aspect_ratio, 0.1f, 1000.0f);
	glutPostRedisplay();
}

void timer_scene(int timestamp_scene) {
	int i;
	for (i = 0; i < NUMMODELS; i++) {
		frame_timer[i] = timestamp_scene % frame_num[i];
	}
	glutPostRedisplay();
	glutTimerFunc(50, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void cleanup(void) {

}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mousepress);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup);
}

void passToShaderFirstTime(void) { // follow OpenGL conventions for initialization
	glUseProgram(phongShader);
	glUniform4f(psLocGlbAmbient, 0.2f, 0.2f, 0.2f, 1.0f);
	for (int ID = 0; ID < NUMLIGHT; ID++) {
		glUniform4fv(psLocLights[ID].pos, 1, lights[ID].pos);
		glUniform3fv(psLocLights[ID].spotDir, 1, lights[ID].spotDir);
		glUniform1i(psLocLights[ID].isOn, lights[ID].isOn);
		glUniform4fv(psLocLights[ID].ambient, 1, lights[ID].ambient);
		glUniform4fv(psLocLights[ID].diffuse, 1, lights[ID].diffuse);
		glUniform4fv(psLocLights[ID].specular, 1, lights[ID].specular);
		glUniform4fv(psLocLights[ID].distFactor, 1, lights[ID].distFactor); // .w != 0.0f for no ligth attenuation
		glUniform1f(psLocLights[ID].spotExpo, lights[ID].spotExpo); // [0.0, 128.0]
		glUniform1f(psLocLights[ID].spotAngle, lights[ID].spotAngle);
	}
	glUseProgram(0);

	glUseProgram(gouraudShader);
	glUniform4f(grLocGlbAmbient, 0.2f, 0.2f, 0.2f, 1.0f);
	for (int ID = 0; ID < NUMLIGHT; ID++) {
		glUniform4fv(grLocLights[ID].pos, 1, lights[ID].pos);
		glUniform3fv(grLocLights[ID].spotDir, 1, lights[ID].spotDir);
		glUniform1i(grLocLights[ID].isOn, lights[ID].isOn);
		glUniform4fv(grLocLights[ID].ambient, 1, lights[ID].ambient);
		glUniform4fv(grLocLights[ID].diffuse, 1, lights[ID].diffuse);
		glUniform4fv(grLocLights[ID].specular, 1, lights[ID].specular);
		glUniform4fv(grLocLights[ID].distFactor, 1, lights[ID].distFactor); // .w != 0.0f for no ligth attenuation
		glUniform1f(grLocLights[ID].spotExpo, lights[ID].spotExpo); // [0.0, 128.0]
		glUniform1f(grLocLights[ID].spotAngle, lights[ID].spotAngle);
	}
	glUseProgram(0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	VMat = glm::lookAt(camPos, camLook, camUp);
	PMat = glm::perspective(zoomCam * TO_RADIAN, 1200.0f/800.0f, 0.1f, 1000.0f);


	glGenTextures(NUMTEXTURE, textures);

	initializeMaterialsAndLights();
	passToShaderFirstTime();
	putLightWC();
	putLightEC();
}

void prepare_scene(void) {
	prepareFloor();
	int i;
	initializeModelsSpace();
	for (i = 0; i < NUMMODELS; i++) prepareModels(i);
	prepareTextures();
}

void initialize_renderer(void) {
	register_callbacks();
	prepareShaderProgram();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char* program_name) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "**************************************************************\n\n");

	initialize_glew();
}


#define N_MESSAGE_LINES 1
void main(int argc, char* argv[]) {
	// Phong Shading
	char program_name[64] = "재미있는 OpenGL 연습해보자~~~ by. 성주희";
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 6);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
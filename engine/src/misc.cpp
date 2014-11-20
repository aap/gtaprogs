#include <renderware.h>
#include "common.h"
#include "atomic.h"
#include "material.h"
#include "mesh.h"
#include "refframe.h"
#include "state.h"
#include "rw.h"

using namespace std;

string basepath = "./";
State *state;
Shader *defaultShader;
Shader *envShader, *specShader, *envSpecShader;
Texture *whitetex, *blacktex;
Material *whitemat;
Mesh *axes;

void
stringToLower(string &s)
{
	for(uint i = 0; i < s.length(); i++)
		s[i] = tolower(s[i]);
}

vector<string>&
splitString(const string &s, char delim, vector<string> &elems)
{
        stringstream ss(s);
        string item;
        while(getline(ss, item, delim))
                elems.push_back(item);
        return elems;
}

Clump*
loadClumpAndTxd(istream &dffstrm, istream &txdstrm)
{
	rw::Clump *clp = new rw::Clump;
	clp->read(dffstrm);
	for(uint i = 0; i < clp->geometryList.size(); i++)
		clp->geometryList[i].cleanUp();
	Clump *clump = fromRwClump(clp);
	INCREF(clump);
	delete clp;

	rw::TextureDictionary *txd = new rw::TextureDictionary;
	txd->read(txdstrm);
	for(uint i = 0; i < txd->texList.size(); i++){
		if(txd->texList[i].platform == rw::PLATFORM_PS2)
			txd->texList[i].convertFromPS2(0x40);
		if(txd->texList[i].platform == rw::PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if(txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}
	TexDict *texdict = fromRwTxd(txd);
	INCREF(texdict);
	delete txd;

	FORALLATOMICS(it, clump)
		(*it)->getMesh()->attachTexDict(texdict);

	DECREF(texdict);
	return clump;
}

Clump*
loadClumpAndTxd(const char *path1, const char *path2)
{
	ifstream dff(path1, ios::binary);
	ifstream txd(path2, ios::binary);
	Clump *c = loadClumpAndTxd(dff, txd);
	dff.close();
	txd.close();
	return c;
}

void
attachMeshToEmptyFrames(Mesh *m, RefFrame *f, Clump *c)
{
	int i = 0;
	FORALLOBJECTS(it, f)
		i++;
	if(i == 0){
		Atomic *a = new Atomic;
		a->setFrame(f);
		a->setMesh(m);
		c->addAtomic(a);
	}
	FORALLCHILDREN(it, f)
		attachMeshToEmptyFrames(m, *it, c);
}

void
initEngine(void)
{
//	glEnable(GL_CULL_FACE);
	glClearColor(0.0824, 0.2118, 0.2588, 1.0);
	glClearDepth(0.0f);
	glDepthRange(1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint whitetexid, blacktexid;
	int white = 0xFFFFFFFF;
	int black = 0xFF000000;
	glGenTextures(1, &whitetexid);
	glGenTextures(1, &blacktexid);
	glBindTexture(GL_TEXTURE_2D, whitetexid);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 1, 1, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, &white);
	glBindTexture(GL_TEXTURE_2D, blacktexid);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 1, 1, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, &black);
	whitetex = new Texture("", whitetexid);
	whitetex->incref();
	blacktex = new Texture("", blacktexid);
	blacktex->incref();

	whitemat = new Material;
	whitemat->incref();
	whitemat->shader = defaultShader;
	whitemat->color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	whitemat->setTexName("");
	whitemat->setTexture(whitetex);

	GLfloat axes_verts[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};
	GLubyte axes_colors[] = {
		255, 255, 255, 255,
		255, 0, 0, 255,
		255, 255, 255, 255,
		0, 255, 0, 255,
		255, 255, 255, 255,
		0, 0, 255, 255
	};
	GLuint axes_indices[] = {
		0, 1, 2, 3, 4, 5
	};
	axes = new Mesh(Mesh::Lines, Mesh::Vertices | Mesh::Colors, 0, 6, 6);
	axes->incref();
	axes->gen();
	axes->setData(Mesh::Vertices, axes_verts);
	axes->setData(Mesh::Colors, axes_colors);
	axes->setIndices(axes_indices);
}

void
initState(State *state)
{
	state->addVar(LIGHTMODEL, new ShaderInt("u_lightmodel"));
	state->addVar(MATCOL, new ShaderVec4("u_matcolor"));
	state->addVar(AMBCOL, new ShaderVec4("u_ambcolor"));
	state->addVar(LIGHTDIR, new ShaderVec3("u_lightdir"));
	state->addVar(NORMALMAT, new ShaderMat3("u_normalmat"));
	state->addVar(PMAT, new ShaderMat4("pmat"));
	state->addVar(MVMAT, new ShaderMat4("mvmat"));
	state->addVar(BONEMATS, new ShaderMatArray("bonemats"));
	state->addVar(TEXTURE0, new ShaderInt("texture0"));
	state->addVar(TEXTURE1, new ShaderInt("texture1"));
	state->addVar(TEXTURE2, new ShaderInt("texture2"));
	state->addVar(SPECMULT, new ShaderFloat("specMult"));
	state->addVar(REFLMULT, new ShaderFloat("reflMult"));

	state->vec4(MATCOL, true)->val = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	state->i(TEXTURE0, true)->val = 0;
	state->i(TEXTURE1, true)->val = 1;
	state->i(TEXTURE2, true)->val = 2;
	MatArray matar = { 0, 0 };
	state->matArray(BONEMATS, true)->val = matar;
}

void
closeEngine(void)
{
        axes->decref();
        whitemat->decref();
        whitetex->decref();
        blacktex->decref();
}

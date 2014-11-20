#include <renderware.h>
#include "common.h"
#include "material.h"
#include "refframe.h"
#include "mesh.h"
#include "atomic.h"
#include "rw.h"

using namespace std;

static Texture*
fromRwTex(rw::NativeTexture *nt)
{
	GLuint texid;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if((nt->rasterFormat & rw::RASTER_MASK) == rw::RASTER_8888 ||
	   !(nt->rasterFormat & (rw::RASTER_PAL8 == rw::RASTER_PAL4)))
		glTexImage2D(GL_TEXTURE_2D, 0, 4, nt->width[0], nt->height[0],
		             0, GL_BGRA, GL_UNSIGNED_BYTE, nt->texels[0]);
	else if((nt->rasterFormat & rw::RASTER_MASK) == rw::RASTER_888 ||
	        !(nt->rasterFormat & (rw::RASTER_PAL8 == rw::RASTER_PAL4)))
		glTexImage2D(GL_TEXTURE_2D, 0, 4, nt->width[0], nt->height[0],
		             0, GL_RGBA, GL_UNSIGNED_BYTE, nt->texels[0]);
	else{
		cout << "can't bind texture " << nt->rasterFormat << endl;
		glDeleteTextures(1, &texid);
		texid = 0;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	Texture *tex = new Texture(nt->name.c_str(), texid);
	tex->alphaDistrib = nt->alphaDistribution;
	return tex;
}

TexDict*
fromRwTxd(rw::TextureDictionary *txd)
{
	TexDict *td = new TexDict;
	for(uint i = 0; i < txd->texList.size(); i++)
		td->setTex(i, fromRwTex(&txd->texList[i]));
	return td;
}

static Material*
fromRwMat(rw::Material *m)
{
	vector<string> texnames;
	Vec4 col(m->color[0], m->color[1], m->color[2], m->color[3]);
	col /= 255.0f;
	Shader *shader = defaultShader;

	if(m->hasTex)
		texnames.push_back(m->texture.name);
	if(m->hasMatFx)
		if(m->matFx->type == rw::MATFX_ENVMAP && m->matFx->hasTex2){
			texnames.push_back(m->matFx->tex2.name);
			shader = envShader;
		}
	if(m->hasSpecularMat){
		texnames.push_back(m->specularName);
		shader = envSpecShader;
//		cout << "specular: " << m->specularLevel SP m->specularLevel*128 << endl;
	}
	if(m->hasReflectionMat){
//		cout << "reflection: " << m->reflectionIntensity SP m->reflectionIntensity*128 << endl;
	}

	Material *mat = new Material(texnames.size());
	mat->shader = shader;
	mat->color = col;
	for(uint i = 0; i < texnames.size(); i++)
		mat->setTexName(texnames[i].c_str(), i);
	// not always used, but copied anyway.
	mat->specMult = m->specularLevel;
	mat->reflMult = m->reflectionIntensity;
	return mat;
}

static Mesh*
fromRwGeo(rw::Geometry *g)
{
	Material **matlist = new Material*[g->materialList.size()];
	for(uint i = 0; i < g->materialList.size(); i++){
//		cout << "material " << i << endl;
		matlist[i] = fromRwMat(&g->materialList[i]);
	}

	int prim = (g->faceType == rw::FACETYPE_LIST) ? Mesh::Triangles :
	                                                Mesh::TriangleStrip;
	int comp = Mesh::Vertices;
	if(g->flags & rw::FLAGS_NORMALS)
		comp |= Mesh::Normals;
	if((g->flags & rw::FLAGS_TEXTURED) || (g->flags & rw::FLAGS_TEXTURED2))
		comp |= Mesh::TexCoords;
	if(g->flags & rw::FLAGS_PRELIT)
		comp |= Mesh::Colors;
	Mesh *m = new Mesh(prim, comp, 0, g->vertices.size()/3, g->numIndices, g->numUVs);
	m->gen();
	m->setData(Mesh::Vertices, &g->vertices[0]);
	if(comp & Mesh::Normals)
		m->setData(Mesh::Normals, &g->normals[0]);
	if(comp & Mesh::TexCoords)
		for(uint i = 0; i < g->numUVs; i++)
			m->setData(Mesh::TexCoords, &g->texCoords[i][0], i);
	if(comp & Mesh::Colors)
		m->setData(Mesh::Colors, &g->vertexColors[0]);
	for(uint i = 0; i < g->splits.size(); i++){
		m->setSubMesh(i, g->splits[i].indices.size());
		m->setMat(i, matlist[g->splits[i].matIndex]);
		m->setIndices(&g->splits[i].indices[0], i);
	}
	if(g->hasSkin){
		Skin *skin = new Skin(g->vertices.size()/3, g->boneCount, &g->vertexBoneWeights[0],
		                      &g->vertexBoneIndices[0], &g->inverseMatrices[0]);
		m->skin = skin;
	} 
	delete[] matlist;
	return m;
}

static RefFrame*
fromRwFrame(rw::Frame *f)
{
	RefFrame *frm = new RefFrame;
	Mat4 &mat = frm->getMatrix();
	mat = Mat4::translation(Vec3(f->position)) * Mat4(Mat3(f->rotationMatrix));
	frm->getResetMatrix() = mat;
	frm->name = f->name;
	stringToLower(frm->name);

	if(f->hasHAnim){
		frm->id = f->hAnimBoneId;
		if(f->hAnimBoneCount != 0)
			frm->setHierarchy(new NodeHierarchy(f->hAnimBoneCount,
			                                    &f->hAnimBoneIds[0],
			                                    (int*)&f->hAnimBoneTypes[0]));
	}
	return frm;
}

Clump*
fromRwClump(rw::Clump *c)
{
	RefFrame *root, **frames = new RefFrame*[c->frameList.size()];
	Mesh **geos = new Mesh*[c->geometryList.size()];

	for(uint i = 0; i < c->frameList.size(); i++)
		frames[i] = fromRwFrame(&c->frameList[i]);
	for(uint i = 0; i < c->frameList.size(); i++)
		if(c->frameList[i].parent == -1)
			root = frames[i];
		else
			frames[c->frameList[i].parent]->addChild(frames[i]);

	for(uint i = 0; i < c->geometryList.size(); i++){
//		cout << "geometry " << i << endl;
		geos[i] = fromRwGeo(&c->geometryList[i]);
	}

	Clump *clump = new Clump;
	clump->setFrame(root);

	for(uint i = 0; i < c->atomicList.size(); i++){
//		cout << frames[c->atomicList[i].frameIndex]->name SP
//		        c->atomicList[i].frameIndex SP
//		        c->atomicList[i].geometryIndex << endl;
		Atomic *a = new Atomic;
		a->setMesh(geos[c->atomicList[i].geometryIndex]);
		a->setFrame(frames[c->atomicList[i].frameIndex]);
		clump->addAtomic(a);
	}

	NodeHierarchy *hier = 0, *tmp;
	for(uint i = 0; i < c->frameList.size(); i++)
		if((tmp = frames[i]->getHierarchy())){
			hier = tmp;
			hier->attach();
//			hier->print(cout);
		}
	if(hier)
		FORALLATOMICS(it, clump)
			if((*it)->getMesh()->skin)
				(*it)->setHierarchy(hier);

	delete[] frames;
	delete[] geos;

	return clump;
}

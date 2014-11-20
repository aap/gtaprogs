#include "common.h"
#include "material.h"
#include "mesh.h"
#include "state.h"

using namespace std;

Mesh::Mesh(void)
 : skin(0), m_components1(0), m_components2(0), m_numTexSets(1),
   m_numVertices(0), m_numIndices(0),
   m_vbo1(0), m_vbo2(0), m_ibo(0), m_wireibo(0)
{
}

Mesh::Mesh(int prim, int comp1, int comp2, int verts, int idx, int numtex)
 : skin(0), m_components1(comp1), m_components2(comp2), m_numTexSets(numtex),
   m_numVertices(verts), m_numIndices(idx),
   m_vbo1(0), m_vbo2(0), m_ibo(0), m_wireibo(0)
{
	const int primtab[] = {
		GL_POINTS,
		GL_LINES,
		GL_LINE_LOOP,
		GL_LINE_STRIP,
		GL_TRIANGLES,
		GL_TRIANGLE_STRIP,
		GL_TRIANGLE_FAN
	};
	SubMesh sm = {idx, 0};
	m_subMeshes.push_back(sm);
	m_prim = primtab[prim];
}

Mesh::~Mesh(void)
{
	THREADCHECK();

	if(m_vbo1) glDeleteBuffers(1, &m_vbo1);
	if(m_vbo2) glDeleteBuffers(1, &m_vbo2);
	if(m_ibo) glDeleteBuffers(1, &m_ibo);
	if(m_wireibo) glDeleteBuffers(1, &m_wireibo);
	for(uint i = 0; i < m_matlist.size(); i++)
		DECREF(m_matlist[i]);
	if(skin)
		delete skin;
}

void
Mesh::gen(void)
{
	THREADCHECK();

	int *cp = &m_components1;
	GLuint *bp = &m_vbo1;
	for(int i = 0; i < 2; i++){
		if(cp[i] == 0)
			continue;
		int size = 0;
		if(cp[i] & Vertices)
			size += m_numVertices*3*sizeof(GLfloat);
		if(cp[i] & Normals)
			size += m_numVertices*3*sizeof(GLfloat);
		if(cp[i] & TexCoords)
			size += m_numTexSets*m_numVertices*2*sizeof(GLfloat);
		if(cp[i] & Colors)
			size += m_numVertices*4*sizeof(GLubyte);
		glGenBuffers(1, &bp[i]);
		glBindBuffer(GL_ARRAY_BUFFER, bp[i]);
		glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STATIC_DRAW);
	}
	glGenBuffers(1, &m_ibo);
	glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ARRAY_BUFFER, m_numIndices*sizeof(GLuint),
	             0, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
Mesh::makewire(void)
{
	THREADCHECK();

	GLuint indices[m_numIndices], newind[m_numIndices*3], *p1, *p2;
	glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, m_numIndices*sizeof(GLuint), indices);
	p1 = indices;
	p2 = newind;
	for(uint i = 0; i < m_subMeshes.size(); i++){
		for(int j = 0; j < m_subMeshes[i].numInd-2; j++){
			if(p1[0] != p1[1] && p1[0] != p1[2] && p1[1] != p1[2]){
				p2[0] = p1[0];
				p2[1] = p1[1];
				p2[2] = p1[2];
				p2 += 3;
			}
			p1 += 1;
		}
		p1 += 2;
	}
	m_numWireinds = p2 - newind;
	glGenBuffers(1, &m_wireibo);
	glBindBuffer(GL_ARRAY_BUFFER, m_wireibo);
	glBufferData(GL_ARRAY_BUFFER, m_numWireinds*sizeof(GLuint),
	             newind, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
Mesh::setSubMesh(uint n, int nidx)
{
	if(n >= m_subMeshes.size())
		m_subMeshes.resize(n+1);
	m_subMeshes[n].numInd = nidx;
}

void
Mesh::setMat(uint n, Material *mat)
{
	bool inlist = false;
	for(uint i = 0; i < m_matlist.size(); i++)
		if(m_matlist[i] == mat){
			cout << "mat already in list\n";
			inlist = true;
			break;
		}
	if(!inlist){
		INCREF(mat);
		m_matlist.push_back(mat);
	}
	if(n >= m_subMeshes.size())
		m_subMeshes.resize(n+1);
	m_subMeshes[n].mat = mat;
}

void
Mesh::setIndices(void *data, uint n)
{
	THREADCHECK();

	int offset = 0, size;
	for(uint i = 0; i < m_subMeshes.size(); i++){
		size = m_subMeshes[i].numInd*sizeof(GLuint);
		if(i == n){
			glBindBuffer(GL_ARRAY_BUFFER, m_ibo);
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			break;
		}
		offset += size;
	}
}

void
Mesh::setData(int type, void *data, int n)
{
	THREADCHECK();

	int *cp = &m_components1;
	GLuint *bp = &m_vbo1;
	for(int i = 0; i < 2; i++){
		if((cp[i] & type) == 0)
			continue;
		glBindBuffer(GL_ARRAY_BUFFER, bp[i]);
		int size, offset = 0;

		if(cp[i] & Vertices){
			size = m_numVertices*3*sizeof(GLfloat);
			if(type == Vertices){
				glBufferSubData(GL_ARRAY_BUFFER, offset,
				                size, data);
				break;
			}
			offset += size;
		}
		if(cp[i] & Normals){
			size = m_numVertices*3*sizeof(GLfloat);
			if(type == Normals){
				glBufferSubData(GL_ARRAY_BUFFER, offset,
				                size, data);
				break;
			}
			offset += size;
		}
		if(cp[i] & TexCoords){
			size = m_numVertices*2*sizeof(GLfloat);
			offset += size*n;
			if(type == TexCoords){
				glBufferSubData(GL_ARRAY_BUFFER, offset,
				                size, data);
				break;
			}
			offset += size*(m_numTexSets-n);
		}
		if(cp[i] & Colors){
			size = m_numVertices*4*sizeof(GLubyte);
			if(type == Colors){
				glBufferSubData(GL_ARRAY_BUFFER, offset,
				                size, data);
				break;
			}
		}
		break;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void
Mesh::draw(void) const
{
	THREADCHECK();
	static int texcoords[] = {
		IN_TEXCOORD0,
		IN_TEXCOORD1
	};

	const int *cp = &m_components1;
	const GLuint *bp = &m_vbo1;
	int lightmodel = 1;
	glVertexAttrib4f(IN_WEIGHTS, 0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 2; i++){
		if(cp[i] == 0)
			continue;
		glBindBuffer(GL_ARRAY_BUFFER, bp[i]);
		int size, offset = 0;

		if(cp[i] & Vertices){
			size = m_numVertices*3*sizeof(GLfloat);
			glEnableVertexAttribArray(IN_VERTEX);
			glVertexAttribPointer(IN_VERTEX, 3, GL_FLOAT,
				GL_FALSE, 0, (GLvoid*) (long) offset);
			offset += size;
		}
		if(cp[i] & Normals){
			size = m_numVertices*3*sizeof(GLfloat);
			glEnableVertexAttribArray(IN_NORMAL);
			glVertexAttribPointer(IN_NORMAL, 3, GL_FLOAT,
				GL_FALSE, 0, (GLvoid*) (long) offset);
			offset += size;
			lightmodel = 0;
		}
		if(cp[i] & TexCoords){
			size = m_numVertices*2*sizeof(GLfloat);
			for(int j = 0; j < m_numTexSets; j++){
				glEnableVertexAttribArray(texcoords[j]);
				glVertexAttribPointer(texcoords[j], 2, GL_FLOAT,
					GL_FALSE, 0, (GLvoid*) (long) offset);
				offset += size;
			}
		}
		if(cp[i] & Colors){
			glEnableVertexAttribArray(IN_COLOR);
			glVertexAttribPointer(IN_COLOR, 4, GL_UNSIGNED_BYTE,
				GL_TRUE, 0, (GLvoid*) (long) offset);
			lightmodel = 1;
		}
	}
	state->i(LIGHTMODEL, true)->val = lightmodel;

	if(skin)
		skin->bind();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	int offset = 0;
	for(uint i = 0; i < m_subMeshes.size(); i++){
		if(m_subMeshes[i].mat)
			m_subMeshes[i].mat->apply();
		else
			whitemat->apply();
		state->upload();
		glDrawElements(m_prim, m_subMeshes[i].numInd, GL_UNSIGNED_INT,
		               (GLvoid*) (long) offset);
	end:
		offset += m_subMeshes[i].numInd*sizeof(GLuint);
	}
	if(m_wireibo){
		glLineWidth(2);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_wireibo);
//		glDisableVertexAttribArray(IN_COLOR);
		glVertexAttrib4f(IN_COLOR, 0.3f, 0.3f, 0.3f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, m_numWireinds, GL_UNSIGNED_INT, 0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glLineWidth(1);
	}

	glDisableVertexAttribArray(IN_VERTEX);
	glDisableVertexAttribArray(IN_NORMAL);
	for(int j = 0; j < m_numTexSets; j++)
		glDisableVertexAttribArray(texcoords[j]);
	glDisableVertexAttribArray(IN_COLOR);
	glDisableVertexAttribArray(IN_WEIGHTS);
	glDisableVertexAttribArray(IN_INDICES);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
Mesh::attachTexDict(TexDict *txd)
{
	for(uint i = 0; i < m_subMeshes.size(); i++){
		Material *m = m_subMeshes[i].mat;
		m->attachTexDict(txd);
	}
}

Mesh::Matlist::iterator
Mesh::matlistBegin(void)
{
	return m_matlist.begin();
}

Mesh::Matlist::iterator
Mesh::matlistEnd(void)
{
	return m_matlist.end();
}

void
Mesh::print(ostream &of) const
{
	of << m_components1 SP m_components2 SP m_numTexSets << endl;
	of << m_numVertices SP m_numIndices << endl;
	of << m_vbo1 SP m_vbo2 SP m_ibo << endl;
	of << m_subMeshes.size() << endl;
}



Skin::Skin(int nverts, int nbones, void *weights, void *indices, float *mats)
 : m_numVertices(nverts), m_numBones(nbones)
{
	THREADCHECK();

	if(nbones > 64)
		cout << "warning: more than 64 bones\n";
	m_invMats = new Mat4[nbones];
	for(int i = 0; i < nbones; i++){
		m_invMats[i] = Mat4(&mats[4*4*i]);
		m_invMats[i].e[0][3] = 0.0f;
		m_invMats[i].e[1][3] = 0.0f;
		m_invMats[i].e[2][3] = 0.0f;
		m_invMats[i].e[3][3] = 1.0f;
	}

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_numVertices*4*(sizeof(GLubyte)+sizeof(GLfloat)),
	             0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_numVertices*4*sizeof(GLubyte), indices);
	glBufferSubData(GL_ARRAY_BUFFER, m_numVertices*4*sizeof(GLubyte),
	                m_numVertices*4*sizeof(GLfloat), weights);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Skin::~Skin(void)
{
	THREADCHECK();

	delete[] m_invMats;
	glDeleteBuffers(1, &m_vbo);
}

int
Skin::getNumBones(void)
{
	return m_numBones;
}

Mat4*
Skin::getMatrices(void)
{
	return m_invMats;
}

void
Skin::bind(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glEnableVertexAttribArray(IN_INDICES);
	glEnableVertexAttribArray(IN_WEIGHTS);
	glVertexAttribPointer(IN_INDICES, 4, GL_UNSIGNED_BYTE,
		GL_FALSE, 0, (GLvoid*) 0);
	glVertexAttribPointer(IN_WEIGHTS, 4, GL_FLOAT,
		GL_FALSE, 0, (GLvoid*) (m_numVertices*4*sizeof(GLubyte)));
}

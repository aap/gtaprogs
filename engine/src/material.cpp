#include "common.h"
#include "material.h"
#include "shader.h"
#include "state.h"

using namespace std;

Texture::Texture(void)
 : alphaDistrib(0), m_texid(0)
{
	name[0] = '\0';
}

Texture::Texture(const char *aname, GLuint texid)
 : m_texid(texid)
{
	strncpy(name, aname, 32);
}

Texture::~Texture(void)
{
	glDeleteTextures(1, &m_texid);
}

void
Texture::setTexId(GLuint texid)
{
	m_texid = texid;
}

GLuint
Texture::getTexId(void)
{
	return m_texid;
}

void
Texture::bind(void)
{
	glBindTexture(GL_TEXTURE_2D, m_texid);
}

void
Texture::print(ostream &of) const
{
	of << "Tex(" << name SP m_texid SP m_refcount << ")";
}



TexDict::~TexDict(void)
{
	for(uint i = 0; i < m_texlist.size(); i++)
		m_texlist[i]->decref();
}

void
TexDict::setTex(uint i, Texture *tex)
{
	tex->incref();
	if(i >= m_texlist.size())
		m_texlist.resize(i+1);
	m_texlist[i] = tex;
}

Texture*
TexDict::getTex(uint i)
{
	if(i >= m_texlist.size())
		return 0;
	return m_texlist[i];
}

Texture*
TexDict::getTex(const char *aname)
{
	for(uint i = 0; i < m_texlist.size(); i++)
		if(strncmp(m_texlist[i]->name, aname, 32) == 0)
			return m_texlist[i];
	return 0;
}

void
TexDict::print(ostream &of) const
{
	for(uint i = 0; i < m_texlist.size(); i++){
		m_texlist[i]->print(of);
		of << endl;
	}
}



Material::Material(int n)
 : shader(0), m_nTexes(n)
{
	m_texnames = (char (*)[32])new char[n*32];
	memset(&m_texnames[0][0], 0, n*32);
	m_texes = new Texture*[n];
	memset(&m_texes[0], 0, n*sizeof(Texture*));
}

Material::~Material(void)
{
	delete[] m_texnames;
	for(int i = 0; i < m_nTexes; i++)
		DECREF(m_texes[i]);
	delete[] m_texes;
}

char*
Material::getTexName(int n)
{
	return m_texnames[n];
}

void
Material::setTexName(const char *name, int n)
{
	strncpy(m_texnames[n], name, 31);
}

Texture*
Material::getTexture(int n)
{
	return m_texes[n];
}

void
Material::setTexture(Texture *tex, int n)
{
	INCREF(tex);
	DECREF(m_texes[n]);
	m_texes[n] = tex;
}

void
Material::apply(void) const
{
	if(shader){
		state->f(SPECMULT)->val = specMult;
		state->f(REFLMULT)->val = reflMult;
		shader->use();
		state->updateLocs(*shader);
	}
	state->vec4(MATCOL, true)->val = color;
	int texunit = GL_TEXTURE0;
	for(int i = 0; i < m_nTexes; i++){
		glActiveTexture(texunit++);
		if(m_texes[i])
			m_texes[i]->bind();
		else
			whitetex->bind();
	}
	glActiveTexture(GL_TEXTURE0);
}

void
Material::attachTexDict(TexDict *txd)
{
	for(int i = 0; i < m_nTexes; i++)
		if(m_texes[i] == 0)
			setTexture(txd->getTex(m_texnames[i]), i);
}

void
Material::print(ostream &of) const
{
//	of << m_texname SP color SP m_tex << endl;
}

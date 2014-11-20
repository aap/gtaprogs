#include "common.h"
#include "shader.h"
#include "state.h"

using namespace std;

ShaderVar::ShaderVar(void)
{
	location = -1;
	name = 0;
	dirty = false;
}

ShaderVar::ShaderVar(const char *name)
{
	location = -1;
	this->name = strdup(name);
	dirty = false;
}

ShaderVar::~ShaderVar(void)
{
	free(name);
}

#define UPLOAD(Type, Func) \
	void\
	Type::upload(void)\
	{\
		Func;\
	}

UPLOAD(ShaderMatArray, glUniformMatrix4fv(location,val.n,GL_FALSE,val.mats))
//UPLOAD(ShaderVec2, glUniform2fv(location, 1, val.ptr()))
UPLOAD(ShaderVec3, glUniform3fv(location, 1, val.ptr()))
UPLOAD(ShaderVec4, glUniform4fv(location, 1, val.ptr()))
UPLOAD(ShaderMat3, glUniformMatrix3fv(location,1,GL_FALSE,val.ptr()))
UPLOAD(ShaderMat4, glUniformMatrix4fv(location,1,GL_FALSE,val.ptr()))
UPLOAD(ShaderFloat, glUniform1f(location, val))
UPLOAD(ShaderInt, glUniform1i(location, val))

#undef UPLOAD


State::~State(void)
{
	for(uint i = 0; i < m_varList.size(); i++)
		delete m_varList[i];
}

#define GETTER(Name, Type) \
	Type*\
	State::Name(int i, bool dirty)\
	{\
		if(dirty)\
			m_varList[i]->dirty = true;\
		return static_cast<Type *>(m_varList[i]);\
	}

GETTER(matArray, ShaderMatArray)
//GETTER(vec2, ShaderVec2)
GETTER(vec3, ShaderVec3)
GETTER(vec4, ShaderVec4)
GETTER(mat3, ShaderMat3)
GETTER(mat4, ShaderMat4)
GETTER(f, ShaderFloat)
GETTER(i, ShaderInt)
GETTER(get, ShaderVar)

#undef GETTER

GLint
State::loc(int i)
{
	return m_varList[i]->location;
}

void
State::addVar(uint index, ShaderVar *var)
{
	if(index >= m_varList.size())
		m_varList.resize(index+1);
	m_varList[index] = var;
}

void
State::updateLocs(Shader &shdr)
{
	for(uint i = 0; i < m_varList.size(); i++)
		if(m_varList[i]->name){
			shdr.getVar(m_varList[i]->name, &m_varList[i]->location,
			            1);
			m_varList[i]->dirty = true;
		}
}

void
State::upload(void)
{
	for(uint i = 0; i < m_varList.size(); i++)
		if(m_varList[i]->dirty)
			m_varList[i]->upload();
}

void
State::dump(void)
{
	for(uint i = 0; i < m_varList.size(); i++)
		if(m_varList[i]->name)
			cout << m_varList[i]->name SP m_varList[i]->location << endl;
}


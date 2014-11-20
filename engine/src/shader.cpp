#include "common.h"
#include "shader.h"

using namespace std;

void
Shader::use(void)
{
	THREADCHECK();

	glUseProgram(m_program);
}

void
Shader::getVar(const char *name, GLint *var, GLint type)
{
	THREADCHECK();

	GLint v = -1;
	if(type == 0)
		v = glGetAttribLocation(m_program, name);
	else if(type == 1)
		v = glGetUniformLocation(m_program, name);
	*var = v;
}

int
Shader::load(const char *vertsrc, const char *fragsrc)
{
	THREADCHECK();

	ifstream shrSrc;
	GLint filelength;
	GLint success;
	GLchar *fragShaderSrc, *vertShaderSrc;

	/* load shader source */
	shrSrc.open(vertsrc);
	if(shrSrc.fail()){
		cerr << "couldn't open " << vertsrc << endl;
		return 1;
	}
	shrSrc.seekg(0, ios::end);
	filelength = shrSrc.tellg();
	shrSrc.seekg(0, ios::beg);
	vertShaderSrc = new char[filelength+1];
	shrSrc.read(vertShaderSrc, filelength);
	vertShaderSrc[filelength] = '\0';
	shrSrc.close();
	
	shrSrc.open(fragsrc);
	if(shrSrc.fail()){
		cerr << "couldn't open " << fragsrc << endl;
		return 1;
	}
	shrSrc.seekg(0, ios::end);
	filelength = shrSrc.tellg();
	shrSrc.seekg(0, ios::beg);
	fragShaderSrc = new char[filelength+1];
	shrSrc.read(fragShaderSrc, filelength);
	fragShaderSrc[filelength] = '\0';
	shrSrc.close();
	

	/* compile vertex shader */
	m_vertshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_vertshader, 1, (const GLchar **)&vertShaderSrc, NULL);
	delete[] vertShaderSrc;
	glCompileShader(m_vertshader);
	glGetShaderiv(m_vertshader, GL_COMPILE_STATUS, &success);
	if(!success){
		cerr << "Error: " << vertsrc << endl;
		printLog(m_vertshader);
		glDeleteProgram(m_vertshader);
		m_vertshader = 0;
		return 1;
	}

	/* compile fragment shader */
	m_fragshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_fragshader, 1, (const GLchar **)&fragShaderSrc, NULL);
	delete[] fragShaderSrc;
	glCompileShader(m_fragshader);
	glGetShaderiv(m_fragshader, GL_COMPILE_STATUS, &success);
	if(!success){
		cerr << "Error: " << fragsrc << endl;
		printLog(m_fragshader);
		glDeleteProgram(m_vertshader);
		glDeleteProgram(m_fragshader);
		m_vertshader = m_fragshader = 0;
		return 1;
	}

	/* link program */
	m_program = glCreateProgram();
	glBindAttribLocation(m_program, IN_VERTEX, "in_vertex");
	glBindAttribLocation(m_program, IN_NORMAL, "in_normal");
	glBindAttribLocation(m_program, IN_TEXCOORD0, "in_texcoord0");
	glBindAttribLocation(m_program, IN_TEXCOORD1, "in_texcoord1");
	glBindAttribLocation(m_program, IN_COLOR, "in_color");
	glBindAttribLocation(m_program, IN_WEIGHTS, "in_weights");
	glBindAttribLocation(m_program, IN_INDICES, "in_indices");
	glAttachShader(m_program, m_vertshader);
	glAttachShader(m_program, m_fragshader);

	glLinkProgram(m_program);
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if(!success){
		cerr << "Error: linking failed\n";
		printLog(m_program);
		glDeleteProgram(m_program);
		glDeleteProgram(m_vertshader);
		glDeleteProgram(m_fragshader);
		m_program = m_vertshader = m_fragshader = 0;
		return 1;
	}

	return 0;
}

void
Shader::destroy(void)
{
	THREADCHECK();

	if(m_program)
		glDeleteProgram(m_program);
	if(m_vertshader)
		glDeleteProgram(m_vertshader);
	if(m_fragshader)
		glDeleteProgram(m_fragshader);
}

void
Shader::printLog(GLuint object)
{
	THREADCHECK();

	GLint len;
	char *log;

	if(glIsShader(object)){
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &len);
		log = new char[len];
		glGetShaderInfoLog(object, len, NULL, log);
	}else if(glIsProgram(object)){
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &len);
		log = new char[len];
		glGetProgramInfoLog(object, len, NULL, log);
	}else{
		cerr << "printLog: Neither shader nor program\n";
		return;
	}
	cout << log;
	delete[] log;
}

Shader::Shader(void)
 : m_program(0), m_vertshader(0), m_fragshader(0)
{
}

Shader::~Shader(void)
{
}


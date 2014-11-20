class Shader
{
private:
	GLint m_program;
	GLint m_vertshader;
	GLint m_fragshader;

public:
	void use(void);
	void getVar(const char *name, GLint *var, GLint type);
	int  load(const char *vertsrc, const char *fragsrc);
	void destroy(void);
	void printLog(GLuint object);
	Shader(void);
	~Shader(void);
};

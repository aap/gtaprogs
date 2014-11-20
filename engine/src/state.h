struct ShaderVar
{
	GLint location;
	char *name;
	bool dirty;

	virtual void upload(void) {};

	ShaderVar(void);
	ShaderVar(const char *name);
	virtual ~ShaderVar(void);
};

struct MatArray
{
	int n;
	float *mats;
};

#define SHADERVAR(Name, Type) \
	struct Name : public ShaderVar {\
		Type val;\
		void upload(void);\
\
		Name(void) : ShaderVar() {};\
		Name(const char *name) : ShaderVar(name) {};\
	};

SHADERVAR(ShaderMatArray, MatArray)
//SHADERVAR(ShaderVec2, Vec2)
SHADERVAR(ShaderVec3, Vec3)
SHADERVAR(ShaderVec4, Vec4)
SHADERVAR(ShaderMat3, Mat3)
SHADERVAR(ShaderMat4, Mat4)
SHADERVAR(ShaderFloat, GLfloat)
SHADERVAR(ShaderInt, GLint)

#undef SHADERVAR

class State
{
private:
	std::vector<ShaderVar *> m_varList;
public:
	~State(void);
	ShaderMatArray *matArray(int i, bool dirty = false);
//	ShaderVec2 *vec2(int i, bool dirty = false);
	ShaderVec3 *vec3(int i, bool dirty = false);
	ShaderVec4 *vec4(int i, bool dirty = false);
	ShaderMat3 *mat3(int i, bool dirty = false);
	ShaderMat4 *mat4(int i, bool dirty = false);
	ShaderFloat *f(int i, bool dirty = false);
	ShaderInt *i(int i, bool dirty = false);
	ShaderVar *get(int i, bool dirty = false);
	GLint loc(int i);
	void addVar(uint index, ShaderVar *var);
	void updateLocs(Shader &shdr);
	void upload(void);

	void dump(void);
};

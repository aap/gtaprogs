class Texture : public RefCounted, public InstCounted<Texture>
{
public:
	char name[32];
	// Needed for alpha test to emulate PS2 behaviour.
	// If bit 0 is set, we have pixels that pass the alpha test,
	//   render as usual. 
	// If bit 1 is set, we have pixels that don't pass the alpha test,
	//   render without writing to the z-buffer.
	// If 0, we don't know anything
	int alphaDistrib;

	Texture(void);
	Texture(const char *name, GLuint texid = 0);
	~Texture(void);
	void setTexId(GLuint texid);
	GLuint getTexId(void);
	void bind(void);

	void print(std::ostream &of) const;
private:
	GLuint m_texid;

	Texture(const Texture&);
	Texture &operator=(const Texture&);
};


class TexDict : public RefCounted, public InstCounted<TexDict>
{
public:
	TexDict(void) {};
	~TexDict(void);
	void setTex(uint i, Texture *tex);
	Texture *getTex(uint i);
	Texture *getTex(const char *name);

	void print(std::ostream &of) const;
private:
	std::vector<Texture *> m_texlist;

	TexDict(const TexDict&);
	TexDict &operator=(const TexDict&);
};


class Material : public RefCounted, public InstCounted<Material>
{
public:
	Vec4 color;
	float reflMult, specMult;
	Shader *shader;

	Material(int n = 1);
	~Material(void);
	char *getTexName(int n = 0);
	void setTexName(const char *name, int n = 0);
	Texture *getTexture(int n = 0);
	void setTexture(Texture *tex, int n = 0);
	void apply(void) const;
	void attachTexDict(TexDict *txd);

	void print(std::ostream&) const;
private:
	int m_nTexes;
	char (*m_texnames)[32];
	Texture **m_texes;

	Material(const Material&);
	Material &operator=(const Material&);
};

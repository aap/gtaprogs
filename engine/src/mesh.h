#ifndef MESH_H
#define MESH_H

#define FORALLMATERIALS(it, mesh)\
	for(Mesh::Matlist::iterator it = mesh->matlistBegin();\
	    it != mesh->matlistEnd(); ++it)

class Mesh : public RefCounted, public InstCounted<Mesh> {
public:
	typedef std::vector<Material*> Matlist;
	Skin *skin;

	enum Components {
		Vertices  = 1,
		Normals   = 2,
		TexCoords = 4,
		Colors    = 8
	};
	enum Prim {
		Points = 0,
		Lines,
		LineLoop,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan
	};
	Mesh(void);
	Mesh(int prim, int comp1, int comp2, int verts, int idx, int numtex = 1);
	~Mesh(void);
	void gen(void);
	void makewire(void);
	void setSubMesh(uint n, int nidx);
	void setMat(uint n, Material *mat);
	void setIndices(void *data, uint n = 0);
	void setData(int type, void *data, int n = 0);
	void draw(void) const;
	void attachTexDict(TexDict *txd);
	Matlist::iterator matlistBegin(void);
	Matlist::iterator matlistEnd(void);

	void print(std::ostream&) const;

private:
	int m_components1, m_components2;
	int m_numTexSets;

	int m_numVertices;
	int m_numIndices;
	int m_numWireinds;

	int m_prim;

	GLuint m_vbo1, m_vbo2;
	GLuint m_ibo;
	GLuint m_wireibo;

	Matlist m_matlist;

	struct SubMesh {
		int numInd;
		Material *mat;
	};
	std::vector<SubMesh> m_subMeshes;

	Mesh(const Mesh&);
	Mesh &operator=(const Mesh&);
};


class Skin {
public:
	Skin(int nverts, int nbones, void *weights, void *indices, float *mats);
	~Skin(void);
	int getNumBones(void);
	Mat4 *getMatrices(void);
	void bind(void);
private:
	int m_numVertices;
	int m_numBones;
	GLuint m_vbo;
	Mat4 *m_invMats;

	Skin(const Skin&);
	Skin &operator=(const Skin&);
};

#endif

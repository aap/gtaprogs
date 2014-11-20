#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <stack>

#include <cstdlib>
#include <cstring>

#include <GL/glew.h>

#include "math/math.h"

#define SP << " " <<

#define PI 3.14159265359f

/* TODO: actually implement this to check if thread has OpenGL context */
#define THREADCHECK()

typedef unsigned int uint;

enum
{
	IN_VERTEX = 0,
	IN_NORMAL,
	IN_TEXCOORD0,
	IN_TEXCOORD1,
	IN_COLOR,
	IN_WEIGHTS,
	IN_INDICES,
};

enum
{
	LIGHTMODEL = 0,
	MATCOL,
	AMBCOL,
	LIGHTDIR,
	NORMALMAT,
	PMAT,
	MVMAT,
	BONEMATS,
	TEXTURE0,
	TEXTURE1,
	TEXTURE2,
	SPECMULT,
	REFLMULT,
	NVARS
};

enum
{
	CloneAtomics = 1,
	CloneMesh
};

#define INCREF(a) if(a) (a)->incref(); else {}
#define DECREF(a) if(a) (a)->decref(); else {}

template <typename T>
class InstCounted
{
protected:
	~InstCounted(void) { s__numInstances--; }
public:
	static unsigned int s__numInstances;
	InstCounted(void) { s__numInstances++; }
};
template <typename T>
unsigned int InstCounted<T>::s__numInstances = 0;


class RefCounted
{
protected:
	int m_refcount;
public:
	virtual ~RefCounted(void) {};
	RefCounted(void) : m_refcount(0) {
		//std::cout << "creating refcounted object\n";
	}
	void incref(void) { m_refcount++; }
	void decref(void) { m_refcount--; if(m_refcount == 0) delete this; }
};

class Shader;
class State;
class Camera;
class Texture;
class Material;
class TexDict;
class Mesh;
class Skin;
class RefFrame;
class NodeHierarchy;
class HierNode;
class Atomic;
class Clump;
class Ped;

extern std::string basepath;
extern State *state;
extern Shader *defaultShader;
extern Shader *envShader, *specShader, *envSpecShader;
extern Texture *whitetex, *blacktex;
extern Material *whitemat;
extern Mesh *axes;

void stringToLower(std::string &s);
std::vector<std::string> &splitString(const std::string &s, char delim, std::vector<std::string> &elems);
//void readConfig(std::istream &in, std::map<std::string, std::string> &config);
Clump *loadClumpAndTxd(std::istream &dffstrm, std::istream &txdstrm);
Clump *loadClumpAndTxd(const char *path1, const char *path2);
void attachMeshToEmptyFrames(Mesh *m, RefFrame *f, Clump *c);
void initEngine(void);
void closeEngine(void);
void initState(State *state);

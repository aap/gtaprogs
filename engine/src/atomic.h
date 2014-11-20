#define FORALLATOMICS(it, clump) \
	for(Clump::Atomiclist::iterator it = clump->atomicsBegin();\
	    it != clump->atomicsEnd(); ++it)


class Atomic : public RefCounted, public InstCounted<Atomic>
{
public:
	Atomic(void);
	~Atomic(void);

	void setMesh(Mesh *mesh);
	Mesh *getMesh(void);
	void setFrame(RefFrame *f, bool remove=true);
	RefFrame *getFrame(void);
	void setHierarchy(NodeHierarchy *hier);
	NodeHierarchy *getHierarchy(void);
	Atomic *clone(int flags = 0);
	void render(void);
private:
	Mesh *m_mesh;
	RefFrame *m_frame;
	NodeHierarchy *m_hier;
	float *m_boneMats;

	Atomic(const Atomic&);
	Atomic &operator=(const Atomic&);
	void updateBoneMats(void);
	void uploadBoneMats(void);
};


class Clump : public RefCounted, public InstCounted<Clump>
{
public:
	typedef std::list<Atomic*> Atomiclist;
	Clump(void);
	~Clump(void);

	void addAtomic(Atomic *atm);
	void removeAtomic(Atomic *atm);
	void setFrame(RefFrame *f);
	RefFrame *getFrame(void);
	Clump *clone(int flags = CloneAtomics);
	void render(void);
	Atomiclist::iterator atomicsBegin(void);
	Atomiclist::iterator atomicsEnd(void);

	Atomic *getAtomic(const char *name);
private:
	RefFrame *m_frame;
	Atomiclist m_atomics;
	typedef std::list<Atomic*>::iterator atmlistiter;

	Clump(const Clump&);
	Clump &operator=(const Clump&);
};

#define FORALLOBJECTS(it, frm) \
	for(std::list<Atomic*>::iterator it = frm->objectsBegin();\
	    it != frm->objectsEnd(); ++it)

#define FORALLCHILDREN(it, frm) \
	for(std::list<RefFrame*>::iterator it = frm->childrenBegin();\
	    it != frm->childrenEnd(); ++it)


class RefFrame : public RefCounted, public InstCounted<RefFrame> {
	friend class Atomic;
public:
	std::string name;
	int id;

	RefFrame(void);
	~RefFrame(void);
	Mat4 &getMatrix(bool makedirty=true);
	Mat4 &getResetMatrix(void);
	const Mat4 &getLTM(void);
	RefFrame *getParent(void);
	void addChild(RefFrame *child);
	void removeChild(RefFrame *child);
	RefFrame *cloneDeep(int flags = 0, Clump *c = 0);
	void setHierarchy(NodeHierarchy *hier);
	NodeHierarchy *getHierarchy(void);
	std::list<Atomic*>::iterator objectsBegin(void);
	std::list<Atomic*>::iterator objectsEnd(void);
	std::list<RefFrame*>::iterator childrenBegin(void);
	std::list<RefFrame*>::iterator childrenEnd(void);

	RefFrame *getChild(const char *name);
	void transform(void);
	void reset(void);
	void print(std::ostream&, int ind = 0);
private:
	RefFrame *m_parent;
	std::list<Atomic*> m_objects;
	std::list<RefFrame*> m_children;
	Mat4 m_xform;
	Mat4 m_reset;
	Mat4 m_ltm;
	bool m_dirty;
	NodeHierarchy *m_hierarchy;

	RefFrame(const RefFrame&);
	RefFrame &operator=(const RefFrame&);
	void addObject(Atomic *atm);
	void removeObject(Atomic *atm);
	void setParent(RefFrame *parent);
	void markdirty(void);
	void updateLTM(void);
};


struct HierNode {
	int id;
	int index;
	int flags;
	RefFrame *frame;
};


class NodeHierarchy {
public:
	RefFrame *frame;
	int numNodes;
	HierNode *nodes;

	NodeHierarchy(int numNodes, int *ids, int *flags);
	~NodeHierarchy(void);
	int getIndex(int id);
	void attach(RefFrame *f = 0);

	void print(std::ostream &os);
private:
//	Mat4 *m_matrices;

	NodeHierarchy(const NodeHierarchy&);
	NodeHierarchy &operator=(const NodeHierarchy&);
};

#include "common.h"
#include "refframe.h"
#include "state.h"
#include "mesh.h"
#include "atomic.h"

using namespace std;

RefFrame::RefFrame(void)
 : id(-1), m_parent(0), m_xform(1.0f), m_dirty(true), m_hierarchy(0)
{
}

RefFrame::~RefFrame(void)
{
	FORALLOBJECTS(it, this){
		(*it)->setFrame(0, false);
		(*it)->decref();
	}
	FORALLCHILDREN(it, this){
		(*it)->setParent(0);
		(*it)->decref();
	}
	delete m_hierarchy;
}

Mat4&
RefFrame::getMatrix(bool markdirty)
{
	if(markdirty)
		this->markdirty();
	return m_xform;
}

Mat4&
RefFrame::getResetMatrix(void)
{
	return m_reset;
}

const Mat4&
RefFrame::getLTM(void)
{
	return m_ltm;
}

RefFrame*
RefFrame::getParent(void)
{
	return m_parent;
}

void
RefFrame::addChild(RefFrame *child)
{
	child->setParent(this);
	child->incref();
	m_children.push_back(child);
}

void
RefFrame::removeChild(RefFrame *child)
{
	child->setParent(0);
	child->decref();
	m_children.remove(child);
}

RefFrame*
RefFrame::cloneDeep(int flags, Clump *c)
{
	Atomic *a;
	RefFrame *f = new RefFrame;
	f->name = name;
	f->getMatrix(true) = getMatrix(false);
	if(flags & CloneAtomics)
		FORALLOBJECTS(it, this){
			a = (*it)->clone(flags);
			a->setFrame(f);
			if(c)
				c->addAtomic(a);
		}
	FORALLCHILDREN(it, this)
		f->addChild((*it)->cloneDeep(flags, c));
	return f;
}

void
RefFrame::setHierarchy(NodeHierarchy *hier)
{
	m_hierarchy = hier;
	if(m_hierarchy)
		m_hierarchy->frame = this;
}

NodeHierarchy*
RefFrame::getHierarchy(void)
{
	return m_hierarchy;
}

std::list<Atomic*>::iterator
RefFrame::objectsBegin(void)
{
	return m_objects.begin();
}

std::list<Atomic*>::iterator
RefFrame::objectsEnd(void)
{
	return m_objects.end();
}

std::list<RefFrame*>::iterator
RefFrame::childrenBegin(void)
{
	return m_children.begin();
}

std::list<RefFrame*>::iterator
RefFrame::childrenEnd(void)
{
	return m_children.end();
}

RefFrame*
RefFrame::getChild(const char *aname)
{
	if(strcmp(aname, name.c_str()) == 0)
		return this;
	FORALLCHILDREN(it, this){
		RefFrame *child = (*it)->getChild(aname);
		if(child)
			return child;
	}
	return 0;
}

void
RefFrame::transform(void)
{
	updateLTM();
	Mat4 &mv = state->mat4(MVMAT, true)->val;
/*
// With Dual Quaternions:
//	DQuat q = DQuat(Quat(1.0f), Quat(m_position/2.0f)) *
//	          DQuat(m_rotation, Quat(0.0f));
//	Mat4 trans = Mat4::transrot(q);
//	mv = mv * trans;

	mv = mv * Mat4::translation(m_position) * Mat4::rotation(m_rotation);
*/
	mv = mv * m_ltm;

	state->mat3(NORMALMAT, true)->val = Mat3(mv);
}

void
RefFrame::reset(void)
{
	m_xform = m_reset;
	m_dirty = true;
	FORALLCHILDREN(it, this)
		(*it)->reset();
}

/*
 * private
 */

void
RefFrame::markdirty(void)
{
	if(m_dirty)
		return;
	m_dirty = true;
	FORALLCHILDREN(it, this)
		(*it)->markdirty();
}

void
RefFrame::updateLTM(void)
{
	if(m_dirty){
		if(m_parent){
			m_parent->updateLTM();
			m_ltm = m_parent->getLTM() * m_xform;
		}else
			m_ltm = m_xform;
	}
}

void
RefFrame::addObject(Atomic *atm)
{
	INCREF(atm);
	m_objects.push_back(atm);
}

void
RefFrame::removeObject(Atomic *atm)
{
	m_objects.remove(atm);
	DECREF(atm);
}

void
RefFrame::setParent(RefFrame *parent)
{
	markdirty();
	m_parent = parent;
}

void
RefFrame::print(std::ostream &os, int ind)
{
	for(int i = 0; i < ind; i++)
		cout << "  ";
	os << name SP m_refcount SP m_objects.size() SP id;
	if(m_hierarchy)
		os SP "<root>";
	os << endl;
	FORALLCHILDREN(it, this)
		(*it)->print(os, ind+1);
}



NodeHierarchy::NodeHierarchy(int anumNodes, int *ids, int *flags)
 : numNodes(anumNodes)
{
	nodes = new HierNode[numNodes];
	for(int i = 0; i < numNodes; i++){
		nodes[i].index = i;
		nodes[i].id = ids[i];
		nodes[i].flags = flags[i];
		nodes[i].frame = 0;
	}
//	m_matrices = new Mat4[numNodes];
}

NodeHierarchy::~NodeHierarchy(void)
{
	delete[] nodes;
//	delete[] m_matrices;
}

int
NodeHierarchy::getIndex(int id)
{
	for(int i = 0; i < numNodes; i++)
		if(nodes[i].id == id)
			return i;
	return -1;
}

void
NodeHierarchy::attach(RefFrame *f)
{
	if(f == 0)
		f = frame;
	int i = getIndex(f->id);
	if(i >= 0)
		nodes[i].frame = f;
	FORALLCHILDREN(it, f)
		attach(*it);
}

void
NodeHierarchy::print(std::ostream &os)
{
	for(int i = 0; i < numNodes; i++){
		os << i SP nodes[i].id SP nodes[i].flags;
		if(nodes[i].frame)
			os SP nodes[i].frame->name;
		os << endl;
	}
}

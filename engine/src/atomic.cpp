#include "common.h"
#include "atomic.h"
#include "mesh.h"
#include "refframe.h"
#include "state.h"

using namespace std;

Atomic::Atomic(void)
 : m_mesh(0), m_frame(0), m_hier(0), m_boneMats(0)
{
}

Atomic::~Atomic(void)
{
	DECREF(m_mesh);
	delete[] m_boneMats;
}

void
Atomic::setMesh(Mesh *mesh)
{
	INCREF(mesh);
	DECREF(m_mesh);
	m_mesh = mesh;
}

Mesh*
Atomic::getMesh(void)
{
	return m_mesh;
}

void
Atomic::setFrame(RefFrame *f, bool remove)
{
	if(f == m_frame)
		return;
	if(f)
		f->addObject(this);
	if(m_frame && remove)
		m_frame->removeObject(this);
	m_frame = f;
}

RefFrame*
Atomic::getFrame(void)
{
	return m_frame;
}

void
Atomic::setHierarchy(NodeHierarchy *hier)
{
	delete[] m_boneMats;
	m_hier = hier;
	if(m_hier)
		m_boneMats = new float[4*4*m_hier->numNodes];
}

NodeHierarchy*
Atomic::getHierarchy(void)
{
	return m_hier;
}

Atomic*
Atomic::clone(int flags)
{
	Atomic *atm = new Atomic;
	atm->setMesh(m_mesh);
	return atm;
}

void
Atomic::render(void)
{
	Mat4 mv = state->mat4(MVMAT, false)->val;
	if(m_hier)
		uploadBoneMats();
	m_frame->transform();
	m_mesh->draw();
	state->mat4(MVMAT, true)->val = mv;
}

void
Atomic::updateBoneMats(void)
{
	Skin *skin = m_mesh->skin;
	if(skin == 0){
		cout << "mesh has no skin\n";
		return;
	}
	if(skin->getNumBones() != m_hier->numNodes){
		cout << "unequal number of bones\n";
		return;
	}
	float *dest = m_boneMats;
	Mat4 *invmats = skin->getMatrices();
	stack<Mat4> matStack;
	matStack.push(Mat4(1.0));
	for(int i = 0; i < m_hier->numNodes; i++){
		HierNode *n = &m_hier->nodes[i];
		if(n->flags & 2)
			matStack.push(matStack.top());
		if(n->frame)
			matStack.top() *= n->frame->getMatrix(false);
		memcpy(dest, (matStack.top()*invmats[i]).ptr(), 4*4*sizeof(float));
		dest += 4*4;
		if(n->flags & 1)
			matStack.pop();
	}
}

void
Atomic::uploadBoneMats(void)
{
	updateBoneMats();
	state->matArray(BONEMATS, true)->val.n = m_hier->numNodes;
	state->matArray(BONEMATS, true)->val.mats = m_boneMats;
}

/*
 * Clump
 */

Clump::Clump(void)
 : m_frame(0)
{
}

Clump::~Clump(void)
{
	DECREF(m_frame);
	for(atmlistiter it = m_atomics.begin(); it != m_atomics.end(); it++)
		(*it)->decref();
}

void
Clump::addAtomic(Atomic *atm)
{
	if(atm){
		INCREF(atm);
		m_atomics.push_back(atm);
	}
}

void
Clump::removeAtomic(Atomic *atm)
{
	if(atm){
		m_atomics.remove(atm);
		DECREF(atm);
	}
}

void
Clump::setFrame(RefFrame *f)
{
	INCREF(f);
	DECREF(m_frame);
	m_frame = f;
}

RefFrame*
Clump::getFrame(void)
{
	return m_frame;
}

Clump*
Clump::clone(int flags)
{
	Clump *c = new Clump;
	c->setFrame(getFrame()->cloneDeep(flags, c));
	return c;
}

void
Clump::render(void)
{
	for(atmlistiter it = m_atomics.begin(); it != m_atomics.end(); it++)
		(*it)->render();
}

Atomic*
Clump::getAtomic(const char *name)
{
	for(atmlistiter it = m_atomics.begin(); it != m_atomics.end(); it++)
		if(strcmp((*it)->getFrame()->name.c_str(), name) == 0)
			return *it;
	return 0;
}

	
Clump::Atomiclist::iterator
Clump::atomicsBegin(void)
{
	return m_atomics.begin();
}

Clump::Atomiclist::iterator
Clump::atomicsEnd(void)
{
	return m_atomics.end();
}

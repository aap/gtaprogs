#include "common.h"
#include "atomic.h"
#include "refframe.h"
#include "animation.h"
#include "ped.h"

using namespace std;

AnimPackage *PedState::tmp_anpk = 0;

Ped::Ped(void)
: animGroup(0), m_clump(0), m_animRoot(0), m_controller(0)
{
}

Ped::~Ped(void)
{
	delete m_controller;
	DECREF(m_clump);
}

void
Ped::initController(void)
{
	m_controller = new AnimationController(m_animRoot, this);
}

static NodeHierarchy*
findHier(RefFrame *f)
{
	NodeHierarchy *hier;
	if((hier = f->getHierarchy()))
		return hier;
	FORALLCHILDREN(it, f){
		if((hier = findHier(*it)))
			return hier;
	}
	return 0;
}

void
Ped::setClump(Clump *clump)
{
	DECREF(m_clump);
	m_animRoot = 0;
	m_clump = clump;
	if(m_clump == 0)
		return;
	INCREF(m_clump);
	NodeHierarchy *hier = findHier(m_clump->getFrame());
	if(hier)
		m_animRoot = hier->frame;
	else
		FORALLCHILDREN(it, m_clump->getFrame()){
			m_animRoot = *it;
			break;
		}

	m_clump->getFrame()->getMatrix() = Mat4(1.0f);
	m_clump->getFrame()->getMatrix() *=Mat4::translation(Vec3(-2.0f, 0.0f, 0.0f));

	Atomic *atm;
/*
	atm = new Atomic;
	atm->setFrame(m_clump->getFrame());
	atm->setMesh(axes);
	m_clump->addAtomic(atm);

	atm = new Atomic;
	atm->setFrame(m_animRoot);
	atm->setMesh(axes);
	m_clump->addAtomic(atm);
*/

	/* Move skinned geometry above bone hierarchy to avoid
	 * transformation by bone AND frame. */
	hier = 0;
	FORALLATOMICS(it, clump){
		if((hier = (atm = *it)->getHierarchy()))
			break;
	}
	if(hier){
		RefFrame *f = hier->frame->getParent();
		if(f)
			atm->setFrame(f);
		else
			cout << "whaaa: no frame above node hierarchy\n";
	}
//	m_clump->getFrame()->print(cout);
}

Clump*
Ped::getClump(void)
{
	return m_clump;
}

void
Ped::render(void)
{
	m_clump->render();
}

void
Ped::setForward(Vec3 dir)
{
	Vec3 up(0.0f, 0.0f, 1.0f);
	Vec3 right = dir.cross(up);
	Mat4 &m = m_clump->getFrame()->getMatrix();
	m.e[0][0] = right.x;
	m.e[0][1] = right.y;
	m.e[0][2] = right.z;
	m.e[1][0] = dir.x;
	m.e[1][1] = dir.y;
	m.e[1][2] = dir.z;
	m.e[2][0] = up.x;
	m.e[2][1] = up.y;
	m.e[2][2] = up.z;
}

void
Ped::translate(Vec3 v)
{
	if(v.norm() > 0.5)
		cout << v << endl;
	m_clump->getFrame()->getMatrix() *= Mat4::translation(v);
}

void
Ped::addTime(float t)
{
	m_controller->addTime(t);
}

void
Ped::handleInput(int key, Vec3 dir)
{
	m_controller->handleInput(key, dir);
}

// #######

AnimationController::AnimationController(RefFrame *root, Ped *ped)
 : root(root), ped(ped)
{
	/* order is important, same as in enum */
	states.push_back(new PedTransition(this));
	states.push_back(new PedIdle(this));
	states.push_back(new PedStart(this));
	states.push_back(new PedMotion(this));

	m_currentState = 0;
	doTransition(-1, Idle);
}

AnimationController::~AnimationController(void)
{
	for(uint i = 0; i < states.size(); i++)
		delete states[i];
}

void
AnimationController::addTime(float t)
{
	IfpFrame f;
	m_currentState->anim->getFrame(&f, root->name, root->id);
	Vec3 lastPos = f.pos;

	bool skip = m_currentState->addTime(t);
	m_currentState->anim->apply(root);

	m_currentState->anim->getFrame(&f, root->name, root->id);

	// remove root bone movement from root
	if(m_currentState->doRootMotion || m_currentState==states[Transition]){
		Mat4 &m = root->getMatrix();
		m = Mat4::translation(Vec3(0.0f, -f.pos.y, 0.0f)) * m;
	}
	// add root bone movement to ped
	if(m_currentState->doRootMotion){
		if(skip)
			lastPos -= m_currentState->anim->getRootPosDelta();
		ped->translate(Vec3(0.0f, (f.pos-lastPos).y, 0.0f));
	}
}

void
AnimationController::handleInput(int key, Vec3 dir)
{
	m_currentState->handleInput(key, dir);
}

void
AnimationController::doTransition(int type, int next, float time)
{
	if(type < 0){
		if(m_currentState)
			m_currentState->exit();
		m_currentState = states[next];
		m_currentState->enter();
	}else{
		PedTransition *trans = (PedTransition*)states[Transition];
		trans->transType = type;
		trans->prev = m_currentState;
		trans->next = states[next];
		trans->time = time;

		if(m_currentState)
			m_currentState->exit();
		m_currentState = trans;
		m_currentState->enter();
	}
}

bool
PedState::addTime(float t)
{
	float time = anim->getTime();
	anim->addTime(t);
	return anim->getLoop() && time > anim->getTime();
}

void
PedState::enter(void)
{
	anim->reset();
}


// #######

PedTransition::PedTransition(AnimationController *ctl)
 : PedState(ctl)
{
	doRootMotion = false;

	type = AnimationController::Transition;
	FrameAnimTransition *a = new FrameAnimTransition;
	anim = a;
}

bool
PedTransition::addTime(float t)
{
	anim->addTime(t);
	if(anim->isDone())
		m_ctl->doTransition(-1, next->type);
	return false;
}

void
PedTransition::enter(void)
{
	if(transType == Blend){
		FrameAnimTransition *a = (FrameAnimTransition*)anim;
		a->setAnimation(0, prev->anim);
		a->setAnimation(1, next->anim);
		next->anim->reset();
		a->setDuration(time);
		a->setTime(0);
		a->reset();
	}
}

// #######

PedIdle::PedIdle(AnimationController *ctl)
: PedState(ctl)
{
	doRootMotion = false;

	type = AnimationController::Idle;
	FrameAnimClip *a = new FrameAnimClip;
	a->setAnimation((*tmp_anpk)["idle_stance"]);
	a->extractRootPosDelta(m_ctl->root->name, ctl->root->id);
	a->setLoop(true);
	anim = a;
}

void
PedIdle::handleInput(int input, Vec3 dir)
{
	switch(input){
	// Motion
	case 0:
		if(dir.norm() > 0.01f)
			m_ctl->doTransition(-1, AnimationController::Start);
		return;
	}
}

// #######

PedStart::PedStart(AnimationController *ctl)
: PedState(ctl)
{
	doRootMotion = true;

	type = AnimationController::Start;
	FrameAnimClip *a = new FrameAnimClip;
	a->setAnimation((*tmp_anpk)["walk_start"]);
	a->extractRootPosDelta(m_ctl->root->name, ctl->root->id);
	a->setLoop(false);
	anim = a;
}

void
PedStart::handleInput(int input, Vec3 dir)
{
	switch(input){
	case 0:
		if(dir.norm() <= 0.01f)
			m_ctl->doTransition(-1, AnimationController::Idle);
		else
			m_ctl->ped->setForward(dir / dir.norm());
		return;
	}
}

bool
PedStart::addTime(float t)
{
	anim->addTime(t);
	if(anim->isDone())
		m_ctl->doTransition(-1, AnimationController::Motion);
	return false;
}

// #######

PedMotion::PedMotion(AnimationController *ctl)
: PedState(ctl)
{
	doRootMotion = true;

	type = AnimationController::Motion;
	FrameAnimClip *a0 = new FrameAnimClip;
	a0->setAnimation((*tmp_anpk)["walk_player"]);
	a0->extractRootPosDelta(ctl->root->name, ctl->root->id);
	FrameAnimClip *a1 = new FrameAnimClip;
	a1->setAnimation((*tmp_anpk)["run_player"]);
	a1->extractRootPosDelta(m_ctl->root->name, ctl->root->id);
	FrameAnimBlend *a = new FrameAnimBlend;
	a->setAnimation(0, a0);
	a->setAnimation(1, a1);
	a->setWeight(0.0f);
	a->setLoop(true);
	anim = a;
}

void
PedMotion::handleInput(int input, Vec3 dir)
{
	switch(input){
	// Motion
	case 0:
		if(dir.norm() <= 0.01f)
			m_ctl->doTransition(PedTransition::Blend, AnimationController::Idle, 0.2f);
		else{
			m_ctl->ped->setForward(dir / dir.norm());
			((FrameAnimBlend*)anim)->setWeight(dir.norm());
		}
		return;
	}
}

void
PedMotion::enter(void)
{
	((FrameAnimBlend*)anim)->setWeight(0.0f);
	anim->reset();
}


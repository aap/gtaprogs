#include <limits>
#include "common.h"
#include "refframe.h"
#include "animation.h"

using namespace std;

static map<uint, AnimInterpolatorInfo*> interpolatorInfos;

void
registerInterpolatorScheme(AnimInterpolatorInfo *interpinfo)
{
	interpolatorInfos[interpinfo->type] = interpinfo;
}

Animation::Animation(uint type, int anumFrames, float aduration)
 : numFrames(anumFrames), duration(aduration)
{
	keyframes = customData = 0;
	if(interpolatorInfos.count(type) == 0){
		cerr << "Can't create animation of type " << type << endl;
		return;
	}
	interpInfo = interpolatorInfos[type];
	interpInfo->alloc(&keyframes, &customData, numFrames);
}

Animation::~Animation(void)
{
	interpInfo->free(keyframes, customData, numFrames);
}

/*
 * AnimInterpolator
 */

#define NEXTFRAME()	(m_nextFrame = (char*)m_nextFrame + frameSize)
#define INTERP(a,i)	((AnimFrameHeader*)((char*)(a)->interpFrames + i*(a)->frameSize))

AnimInterpolator::AnimInterpolator(void)
 : m_nextFrame(0), frameSize(0), loop(false), anim(0), time(0.0f), interpFrames(0), numNodes(-1)
{
}

void
AnimInterpolator::setAnimation(Animation *an)
{
	if(anim)
		anim->interpInfo->freeInterp(interpFrames, numNodes);
	anim = an;
	numNodes = anim->numNodes;
	interpFrames = anim->interpInfo->allocInterp(numNodes);
	m_nextFrame = anim->keyframes;
	frameSize = anim->interpInfo->frameSize;
}

void
AnimInterpolator::reset(void)
{
	time = 0.0f;
	m_nextFrame = anim->keyframes;
	for(int i = 0; i < numNodes; i++){
		INTERP(this,i)->interp.key1 = (AnimFrameHeader*)m_nextFrame;
		NEXTFRAME();
	}
	for(int i = 0; i < numNodes; i++){
		INTERP(this,i)->interp.key2 = (AnimFrameHeader*)m_nextFrame;
		NEXTFRAME();
		anim->interpInfo->interpolate(INTERP(this,i), INTERP(this,i)->interp.key1,
		                              INTERP(this,i)->interp.key2, 0.0f);
	}
}

void
AnimInterpolator::addTime(float atime)
{
	if(anim->duration == 0.0f)
		return;
	time += atime;
	if(time > anim->duration){
		if(loop){
			while(time > anim->duration)
				time -= anim->duration;
			reset();
		}else
			time = anim->duration;
	}
	while(1){
		int n = -1;
		float minTime = numeric_limits<float>::max();
		for(int i = 0; i < numNodes; i++){
			AnimFrameHeader *f = INTERP(this,i);
			if(time > f->interp.key2->key.time && 
			   f->interp.key2->key.time < minTime){
				minTime = f->interp.key2->key.time;
				n = i;
			}
		}
		if(n < 0)
			break;
		AnimFrameHeader *f = INTERP(this,n);
		f->interp.key1 = f->interp.key2;
		f->interp.key2 = (AnimFrameHeader*)m_nextFrame;
		NEXTFRAME();
	}
	for(int i = 0; i < numNodes; i++){
		AnimFrameHeader *f = INTERP(this,i);
		float a = (f->interp.key1->key.time - f->interp.key2->key.time);
		if(a != 0.0f)
			a = (f->interp.key1->key.time - time) / a;
		anim->interpInfo->interpolate(f, f->interp.key1, f->interp.key2, a);
	}
}

/*
 * IfpInterpInfo
 */

void
IfpInterpInfo::doRegister(void)
{
	type = 1234;
	frameSize = sizeof(Frame);
	customDataSize = sizeof(map<string, int>);

	registerInterpolatorScheme(this);	
}

void
IfpInterpInfo::apply(void *result, AnimFrameHeader *af)
{
	RefFrame *rf = (RefFrame*)result;
	Frame *f = (IfpFrame*)af;
	if(f->type < 0)
		return;
	Mat4 &mat = rf->getMatrix();
	Vec3 pos(mat.e[3][0], mat.e[3][1], mat.e[3][2]);
	mat = Mat4::rotation(f->rot);
	if(f->type > 0)
		pos = f->pos;
	mat.e[3][0] = pos.x;
	mat.e[3][1] = pos.y;
	mat.e[3][2] = pos.z;
}

void
IfpInterpInfo::blend(AnimFrameHeader *out, AnimFrameHeader *in1,
	             AnimFrameHeader *in2, float a)
{
	Frame *outf = (Frame*)out;
	Frame *inf1 = (Frame*)in1;
	Frame *inf2 = (Frame*)in2;
	outf->rot = inf1->rot.lerp(inf2->rot, a);
	outf->pos = inf1->pos*(1.0f - a) + inf2->pos*a;
	outf->scale = inf1->scale*(1.0f - a) = inf2->scale*a;
	outf->type = inf1->type;
}

void
IfpInterpInfo::interpolate(AnimFrameHeader *out, AnimFrameHeader *in1,
	                   AnimFrameHeader *in2, float a)
{
	Frame *outf = (Frame*)out;
	Frame *inf1 = (Frame*)in1;
	Frame *inf2 = (Frame*)in2;
	outf->rot = inf1->rot.slerp(inf2->rot, a);
	outf->pos = inf1->pos*(1.0f - a) + inf2->pos*a;
	outf->scale = inf1->scale*(1.0f - a) = inf2->scale*a;
	outf->type = inf1->type;
}

void
IfpInterpInfo::alloc(void **kf, void **cd, int n)
{
	*kf = new Frame[n];
	*cd = new Custom;
}

void
IfpInterpInfo::free(void *kf, void *cd, int)
{
	Frame *kp = (Frame*)kf;
	delete[] kp;
	Custom *cp = (Custom*)cd;
	delete cp;
}

void*
IfpInterpInfo::allocInterp(int n)
{
	return new Frame[n];
}

void
IfpInterpInfo::freeInterp(void *f, int)
{
	Frame *p = (Frame*)f;
	delete[] p;
}



void
applyIfpAnim(RefFrame *f, AnimInterpolator *a)
{
	IfpFrame *kf = getIfpFrame(a, f->name, f->id);
	if(kf)
		a->anim->interpInfo->apply(f, kf);

	FORALLCHILDREN(it, f)
		applyIfpAnim(*it, a);
}

IfpFrame*
getIfpFrame(AnimInterpolator *a, string name, int id)
{
	IfpInterpInfo::Custom &c = *(IfpInterpInfo::Custom*)a->anim->customData;

	if(c.names.count(name) != 0)
		return (IfpFrame*) INTERP(a, c.names[name]);
	if(id != -1 && c.ids.count(id) != 0)
		return (IfpFrame*) INTERP(a, c.ids[id]);
	return 0;
}





AnimInterpolatorInfo *FrameAnimation::interpInfo = 0;

bool
FrameAnimation::getLoop(void)
{
	return m_loop;
}

void
FrameAnimation::apply(RefFrame *hier)
{
	IfpFrame f;
	getFrame(&f, hier->name, hier->id);
	interpInfo->apply(hier, &f);
	FORALLCHILDREN(it, hier)
		apply(*it);
}

bool
FrameAnimation::isDone(void)
{
	return !m_loop && getTime() >= getDuration();
}


void
FrameAnimClip::setAnimation(Animation *anim)
{
	m_interp.setAnimation(anim);
	m_interp.reset();
}

void
FrameAnimClip::extractRootPosDelta(std::string name, int id)
{
	IfpFrame f1, f2;
	reset();
	getFrame(&f1, name, id);
	skipToEnd();
	getFrame(&f2, name, id);
	reset();
	m_rootPosDelta = f2.pos - f1.pos;
}

float
FrameAnimClip::getDuration(void)
{
	return m_interp.anim->duration;
}

void
FrameAnimClip::setLoop(bool loop)
{
	m_loop = loop;
	m_interp.loop = loop;
}

void
FrameAnimClip::addTime(float t)
{
	m_interp.addTime(t);
}

float
FrameAnimClip::getTime(void)
{
	return m_interp.time;
}

void
FrameAnimClip::reset(void)
{
	m_interp.reset();
}

void
FrameAnimClip::skipToEnd(void)
{
	m_interp.reset();
	m_interp.addTime(m_interp.anim->duration);
}

void
FrameAnimClip::getFrame(IfpFrame *f, std::string name, int id)
{
	IfpFrame *frm;
	frm = getIfpFrame(&m_interp, name, id);
	f->type = -1;
	if(frm == 0)
		return;
	f->pos = frm->pos;
	f->rot = frm->rot;
	f->scale = frm->scale;
	f->type = frm->type;
}

Vec3
FrameAnimClip::getRootPosDelta(void)
{
	return m_rootPosDelta;
}



FrameAnimBlend::FrameAnimBlend(void)
 : m_weight(0.0f), m_a0(0), m_a1(0)
{
}

void
FrameAnimBlend::setAnimation(int i, FrameAnimation *anim)
{
	if(i == 0){
		delete m_a0;
		m_a0 = anim;
	}else{
		delete m_a1;
		m_a1 = anim;
	}
}

void
FrameAnimBlend::setWeight(float w)
{
	float duration = getDuration();
	float normalTime = getTime()/duration;
	m_weight = w;
	reset();
	addTime(normalTime*getDuration());
}

float
FrameAnimBlend::getWeight(void)
{
	return m_weight;
}

FrameAnimBlend::~FrameAnimBlend(void)
{
	delete m_a0;
	delete m_a1;
}

float
FrameAnimBlend::getDuration(void)
{
	return m_a0->getDuration() * (1.0f - m_weight) +
	       m_a1->getDuration() * m_weight;
}

void
FrameAnimBlend::setLoop(bool loop)
{
	m_loop = loop;
	m_a0->setLoop(loop);
	m_a1->setLoop(loop);
}

void
FrameAnimBlend::addTime(float t)
{
	float duration = getDuration();
	m_a0->addTime(t*m_a0->getDuration()/duration);
	m_a1->addTime(t*m_a1->getDuration()/duration);
}

float
FrameAnimBlend::getTime(void)
{
	return m_a0->getTime() * (1.0f - m_weight) +
	       m_a1->getTime() * m_weight;
}

void
FrameAnimBlend::reset(void)
{
	m_a0->reset();
	m_a1->reset();
}

void
FrameAnimBlend::skipToEnd(void)
{
	m_a0->skipToEnd();
	m_a1->skipToEnd();
}

void
FrameAnimBlend::getFrame(IfpFrame *f, std::string name, int id)
{
	IfpFrame *f0, *f1;
	IfpFrame frames[2];
	m_a0->getFrame(&frames[0], name, id);
	m_a1->getFrame(&frames[1], name, id);
	f0 = &frames[0];
	f1 = &frames[1];

	f->type = -1;
	if(f0 == 0){
		f0 = f1;
		goto single;
	}
	if(f1 == 0)
		goto single;

	interpInfo->blend(f, f0, f1, m_weight);
	return;

single:
	if(f0 == 0)
		return;
	f->pos = f0->pos;
	f->rot = f0->rot;
	f->scale = f0->scale;
	f->type = f0->type;
}

Vec3
FrameAnimBlend::getRootPosDelta(void)
{
	return m_a0->getRootPosDelta() * (1.0f - m_weight) +
	       m_a1->getRootPosDelta() * m_weight;
}


FrameAnimTransition::FrameAnimTransition(void)
 : m_duration(0.0f), m_time(0.0f), m_a0(0), m_a1(0)
{
}

void
FrameAnimTransition::setAnimation(int i, FrameAnimation *anim)
{
	if(i == 0)
		m_a0 = anim;
	else
		m_a1 = anim;
}

void
FrameAnimTransition::setDuration(float t)
{
	m_duration = t;
}

void
FrameAnimTransition::setTime(float t)
{
	m_time = t;
}

float
FrameAnimTransition::getDuration(void)
{
	return m_duration;
}

void
FrameAnimTransition::addTime(float t)
{
	m_time += t;
}

float
FrameAnimTransition::getTime(void)
{
	return m_time;
}

void
FrameAnimTransition::reset(void)
{
	m_time = 0;
}

void
FrameAnimTransition::skipToEnd(void)
{
	m_time = m_duration;
}

void
FrameAnimTransition::getFrame(IfpFrame *f, std::string name, int id)
{
	float weight = m_time/m_duration;

	IfpFrame *f0, *f1;
	IfpFrame frames[2];
	m_a0->getFrame(&frames[0], name, id);
	m_a1->getFrame(&frames[1], name, id);
	f0 = &frames[0];
	f1 = &frames[1];

	f->type = -1;
	if(f0 == 0){
		f0 = f1;
		goto single;
	}
	if(f1 == 0)
		goto single;

	interpInfo->blend(f, f0, f1, weight);
	return;

single:
	if(f0 == 0)
		return;
	f->pos = f0->pos;
	f->rot = f0->rot;
	f->scale = f0->scale;
	f->type = f0->type;
}

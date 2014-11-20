class AnimationController;
class PedState;

class Ped
{
public:
	int animGroup;

	Ped(void);
	~Ped(void);
	void initController(void);
	void setClump(Clump *clump);
	Clump *getClump(void);
	void render(void);

	void addTime(float t);
	void handleInput(int key, Vec3 dir);

	void translate(Vec3 v);
	// TODO: handle in PedState or AnimationController?
	void setForward(Vec3 dir);
private:
	Clump *m_clump;
	RefFrame *m_animRoot;
	AnimationController *m_controller;

	Ped(const Ped&);
	Ped &operator=(const Ped&);
};

class AnimationController
{
public:
	enum {
		Transition = 0,
		Idle,
		Start,
		Motion
	};
	std::vector<PedState*> states;
	RefFrame *root;
	Ped *ped;

	AnimationController(RefFrame *root, Ped *ped);
	~AnimationController(void);
	void addTime(float t);
	void handleInput(int key, Vec3 dir);
	void doTransition(int type, int next, float time = 0.0f);

private:
	PedState *m_currentState;
	// next state/transition
};

class PedState
{
public:
	int type;
	bool doRootMotion;
	FrameAnimation *anim;

	PedState(AnimationController *ctl)
	 : m_ctl(ctl) {};
	virtual ~PedState(void) { delete anim; }
	virtual void handleInput(int, Vec3) {};
	virtual bool addTime(float t);
	virtual void enter(void);
	virtual void exit(void) {};

	// TODO: fucking remove this
	static AnimPackage *tmp_anpk;

protected:
	// std::vector transitions
	AnimationController *m_ctl;
};

class PedTransition : public PedState
{
public:
	enum {
		Blend = 0,
		Cross
	};
	int transType;
	float time;
	PedState *prev, *next;

	PedTransition(AnimationController *ctl);
	bool addTime(float t);
	void enter(void);
private:
};

class PedIdle : public PedState
{
public:
	PedIdle(AnimationController *ctl);
private:
	void handleInput(int, Vec3);
};

class PedStart : public PedState
{
public:
	PedStart(AnimationController *ctl);
private:
	void handleInput(int, Vec3);
	bool addTime(float t);
};

class PedMotion : public PedState
{
public:
	PedMotion(AnimationController *ctl);
private:
	void handleInput(int, Vec3);
	void enter(void);
};

struct AnimFrameHeader
{
	union {
	struct {
		AnimFrameHeader *prev;
		float time;
	} key;
	struct {
		AnimFrameHeader *key1;
		AnimFrameHeader *key2;
	} interp;
	};
};


class AnimInterpolatorInfo
{
public:
	uint type;
	uint frameSize;
	uint customDataSize;

	virtual void apply(void *result, AnimFrameHeader *f) = 0;
	virtual void blend(AnimFrameHeader *out, AnimFrameHeader *in1,
	                   AnimFrameHeader *in2, float a) = 0;
	virtual void interpolate(AnimFrameHeader *out, AnimFrameHeader *in1,
	                         AnimFrameHeader *in2, float a) = 0;
//	virtual void add(void) {};
//	virtual void mulrecip(void) {};

	virtual void alloc(void **kf, void **cd, int n) = 0;
	virtual void free(void *kf, void *cd, int n) = 0;
	virtual void *allocInterp(int n) = 0;
	virtual void freeInterp(void *f, int n) = 0;
};


class Animation
{
public:
	AnimInterpolatorInfo *interpInfo;
	int numFrames;
	int numNodes;
	float duration;
	void *keyframes;
	void *customData;

	Animation(uint type, int numFrames, float duration);
	~Animation(void);

private:
	Animation(const Animation&);
	Animation &operator=(const Animation&);
};


class AnimInterpolator
{
private:
	void *m_nextFrame;
public:
	uint frameSize;
	bool loop;

	Animation *anim;
	float time;
	void *interpFrames;
	int numNodes;

	AnimInterpolator(void);
	void setAnimation(Animation *an);
	void reset(void);
	void addTime(float time);
};

void registerInterpolatorScheme(AnimInterpolatorInfo *interpinfo);


struct IfpInterpInfo : public AnimInterpolatorInfo
{
	struct Frame : AnimFrameHeader
	{
		int type;
		Quat rot;
		Vec3 pos;
		Vec3 scale;
	};
	struct Custom
	{
		std::map<std::string, int> names;
		std::map<int, int> ids;
	};
	void doRegister(void);

	void apply(void *result, AnimFrameHeader *f);
	void blend(AnimFrameHeader *out, AnimFrameHeader *in1,
	           AnimFrameHeader *in2, float a);
	void interpolate(AnimFrameHeader *out, AnimFrameHeader *in1,
	                 AnimFrameHeader *in2, float a);

	void alloc(void **kf, void **cd, int n);
	void free(void *kf, void *cd, int n);
	void *allocInterp(int n);
	void freeInterp(void *f, int n);
};
typedef IfpInterpInfo::Frame IfpFrame;

typedef std::map<std::string, Animation*> AnimPackage;

AnimPackage readIfp(std::ifstream &ifp);

void applyIfpAnim(RefFrame *f, AnimInterpolator *a);
IfpFrame *getIfpFrame(AnimInterpolator *a, std::string name, int id = -1);


class FrameAnimation
{
public:
	static AnimInterpolatorInfo *interpInfo;

	virtual ~FrameAnimation(void) {};
	virtual float getDuration(void) = 0;
	virtual void setLoop(bool loop) = 0;
	bool getLoop(void);
	virtual void addTime(float t) = 0;
	virtual float getTime(void) = 0;
	virtual void reset(void) = 0;
	virtual void skipToEnd(void) = 0;
	virtual void getFrame(IfpFrame *f, std::string name, int id = -1) = 0;
	virtual Vec3 getRootPosDelta(void) { return Vec3(0.0f, 0.0f, 0.0f); };
	void apply(RefFrame *hier);
	bool isDone(void);
//	TODO?
//	virtual void pause(void);

protected:
	bool m_loop;
};

class FrameAnimClip: public FrameAnimation
{
public:
	void setAnimation(Animation *anim);
	void extractRootPosDelta(std::string name, int id = -1);

	float getDuration(void);
	void setLoop(bool loop);
	void addTime(float t);
	float getTime(void);
	void reset(void);
	void skipToEnd(void);
	void getFrame(IfpFrame *f, std::string name, int id = -1);
	Vec3 getRootPosDelta(void);
private:
	AnimInterpolator m_interp;
	Vec3 m_rootPosDelta;
};

class FrameAnimBlend: public FrameAnimation
{
public:
	FrameAnimBlend(void);
	void setAnimation(int i, FrameAnimation *anim);
	void setWeight(float w);
	float getWeight(void);

	~FrameAnimBlend(void);
	float getDuration(void);
	void setLoop(bool loop);
	void addTime(float t);
	float getTime(void);
	void reset(void);
	void skipToEnd(void);
	void getFrame(IfpFrame *f, std::string name, int id = -1);
	Vec3 getRootPosDelta(void);
private:
	float m_weight;
	FrameAnimation *m_a0, *m_a1;
};

class FrameAnimTransition: public FrameAnimation
{
public:
	FrameAnimTransition(void);
	void setAnimation(int i, FrameAnimation *anim);
	void setDuration(float t);
	void setTime(float t);

	float getDuration(void);
	void setLoop(bool) {};
	void addTime(float t);
	float getTime(void);
	void reset(void);
	void skipToEnd(void);
	void getFrame(IfpFrame *f, std::string name, int id = -1);

private:
	float m_duration;
	float m_time;
	FrameAnimation *m_a0, *m_a1;
};

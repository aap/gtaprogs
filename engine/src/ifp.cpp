#include <limits>
#include <renderware.h>
#include "common.h"
#include "animation.h"
#include "rw.h"

using namespace std;
using namespace rw;

/*
 * IFP
 */

enum {
	ANP3 = 0x33504e41,
	ANPK = 0x4b504e41,
	INFO = 0x4f464e49,
	NAME = 0x454d414e,
	DGAN = 0x4e414744,
	CPAN = 0x4e415043,
	ANMS = 0x4d494e41
};

enum {
	KR00 = 0x3030524b,
	KRT0 = 0x3054524b,
	KRTS = 0x5354524b
};

struct IfpNode {
	string name;
	int boneId;
	vector<IfpInterpInfo::Frame> frames;
};

struct IfpAnim {
	string name;
	float duration;
	vector<IfpNode> nodes;
};

struct IfpAnimPackage {
	string name;
	vector<IfpAnim> anims;
};

#define READSECTION(x)\
	fourcc = readUInt32(ifp);\
	if(fourcc != x){\
		cerr << "error: no " << hex << x << " found at " << hex << ifp.tellg() << " (" << fourcc << ")\n";\
		exit(1);\
	}\
	size = readUInt32(ifp);

static void
readIfpFrame1(IfpInterpInfo::Frame &frame, uint32 fourcc, ifstream &ifp)
{
	float data[4];

	ifp.read((char*)&data, 4*sizeof(float));
	frame.rot.w = data[3];
	frame.rot.x = -data[0];
	frame.rot.y = -data[1];
	frame.rot.z = -data[2];
	frame.type = 0;
	if(fourcc == KR00)
		goto end;
	ifp.read((char*)&data, 3*sizeof(float));
	frame.pos.x = data[0];
	frame.pos.y = data[1];
	frame.pos.z = data[2];
	frame.type = 1;
	if(fourcc == KRT0)
		goto end;
	ifp.read((char*)&data, 3*sizeof(float));
	frame.scale.x = data[0];
	frame.scale.y = data[1];
	frame.scale.z = data[2];
	frame.type = 2;
	if(fourcc == KRTS)
		goto end;
	cerr << "unknown frame type " << hex << fourcc << endl;
end:
	frame.key.time = readFloat32(ifp);
}

static void
readIfpFrame3(IfpInterpInfo::Frame &frame, uint32 frmType, ifstream &ifp)
{
	int16 data[5];
	ifp.read((char*)&data, 5*sizeof(int16));
	frame.rot.x = data[0]/4096.0f;
	frame.rot.y = data[1]/4096.0f;
	frame.rot.z = data[2]/4096.0f;
	frame.rot.w = data[3]/4096.0f;
	frame.key.time = data[4]/60.0f;
	frame.type = 0;
	if(frmType == 4){
		ifp.read((char*)&data, 3*sizeof(int16));
		frame.pos.x = data[0]/1024.0f;
		frame.pos.y = data[1]/1024.0f;
		frame.pos.z = data[2]/1024.0f;
		frame.type = 1;
	}
}

static float
readIfpNode1(IfpNode &node, ifstream &ifp)
{
	uint32 fourcc, size;

	READSECTION(CPAN);
	READSECTION(ANMS);
	char *buf = new char[28];
	ifp.read(buf, 28);
	node.name = buf;
	delete[] buf;
	uint32 frames = readInt32(ifp);
	int32 unknown = readInt32(ifp); (void)unknown;
	int32 prev = readInt32(ifp); (void)prev;
	node.boneId = readInt32(ifp);
	ifp.seekg(size-28-4*sizeof(int32), ios::cur);
//		cout << "	" << node.name SP dec << frames SP unknown SP next SP node.boneId << endl;

	if(frames == 0)
		return 0.0;

	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);
	node.frames.resize(frames);
	for(uint32 i = 0; i < frames; i++)
		readIfpFrame1(node.frames[i], fourcc, ifp);
	return node.frames[node.frames.size()-1].key.time;
}

static float
readIfpNode3(IfpNode &node, ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	node.name = buf;
	uint32 frmType = readUInt32(ifp);
	uint32 frames = readUInt32(ifp);
	node.boneId = readInt32(ifp);
//		cout << "	" << node.name SP node.boneId << endl;
	node.frames.resize(frames);
	for(uint32 i = 0; i < frames; i++)
		readIfpFrame3(node.frames[i], frmType, ifp);
	return node.frames[node.frames.size()-1].key.time;
}

static void
readIfpAnim1(IfpAnim &anim, ifstream &ifp)
{
	uint32 fourcc, size;

	READSECTION(NAME);
	size = (size+0x3) & ~0x3;
	char *buf = new char[size];
	ifp.read(buf, size);
	anim.name = buf;
//		cout << anim.name << endl;
	delete[] buf;

	READSECTION(DGAN);
	uint32 end = size + ifp.tellg();

	READSECTION(INFO);
	uint32 numNodes = readUInt32(ifp);
	size = ((size-sizeof(uint32))+0x3) & ~0x3;
	ifp.seekg(size, ios::cur);

	anim.duration = 0.0;
	for(uint32 i = 0; i < numNodes && ifp.tellg() < end; i++){
		anim.nodes.resize(anim.nodes.size()+1);
		float last = readIfpNode1(anim.nodes[i], ifp);
		if(last > anim.duration)
			anim.duration = last;
	}
}

static void
readIfpAnim3(IfpAnim &anim, ifstream &ifp)
{
	char buf[24];
	ifp.read(buf, 24);
	anim.name = buf;
//		cout << anim.name << endl;
	uint32 numNodes = readUInt32(ifp);
	ifp.seekg(4, ios::cur);	// frame Size
	ifp.seekg(4, ios::cur);

	anim.duration = 0.0f;
	anim.nodes.resize(numNodes);
	for(uint32 i = 0; i < numNodes; i++){
		float last = readIfpNode3(anim.nodes[i], ifp);
		if(last > anim.duration)
			anim.duration = last;
	}
}

static void
readIfpAnimPackage(IfpAnimPackage &anpk, ifstream &ifp)
{
	uint32 fourcc, size;
	fourcc = readUInt32(ifp);
	size = readUInt32(ifp);
	if(fourcc == ANPK){
		READSECTION(INFO);
		uint32 numAnims = readUInt32(ifp);
		anpk.anims.resize(numAnims);
		size = ((size-sizeof(uint32))+0x3) & ~0x3;
		char *buf = new char[size];
		ifp.read(buf, size);
		anpk.name = buf;
		delete[] buf;
//		cout << numAnims << " animations in " << anpk.name << endl;
		for(uint32 i = 0; i < numAnims; i++)
			readIfpAnim1(anpk.anims[i], ifp);
	}else if(fourcc == ANP3){
		char buf[24];
		ifp.read(buf, 24);
		anpk.name = buf;
		uint32 numAnims = readUInt32(ifp);
//		cout << numAnims << " animations in " << anpk.name << endl;
		anpk.anims.resize(numAnims);
		for(uint32 i = 0; i < numAnims; i++)
			readIfpAnim3(anpk.anims[i], ifp);
	}else{
		cout << "unknown ifp file\n";
	}
}

Animation*
convertIfpAnim(IfpAnim &ianim)
{
//	typedef IfpInterpInfo::Frame KeyFrame;

	int numNodes = ianim.nodes.size();
	int totalFrames = 0;
	for(int i = 0; i < numNodes; i++){
		IfpNode &n = ianim.nodes[i];
		if(n.frames.size() < 2){
//			cout << ianim.name SP n.name SP n.frames.size() << endl;
			if(n.frames.size() == 1){
				n.frames.push_back(n.frames[0]);
				n.frames[1].key.time = ianim.duration;
			}
		}
		int size = n.frames.size();
		if(n.frames.size() != 0 && n.frames[size-1].key.time != ianim.duration){
			n.frames.push_back(n.frames[size-1]);
			n.frames[size].key.time = ianim.duration;
			size++;
		}
		totalFrames += size;
	}

	Animation *anim = new Animation(1234, totalFrames, ianim.duration);
	IfpFrame *keyframes = (IfpFrame*)anim->keyframes;
	IfpInterpInfo::Custom &c = *(IfpInterpInfo::Custom*)anim->customData;
//	map<string, int> &nodemap = *(map<string, int>*)anim->customData;
	anim->numNodes = numNodes;

	IfpFrame **prevKey = new IfpFrame*[numNodes];
	IfpFrame **currentKey = new IfpFrame*[numNodes];
	int *numFrames = new int[numNodes];

	memset(prevKey, 0, sizeof(IfpFrame*)*numNodes);
	for(int i = 0, j = 0; i < numNodes; i++){
		numFrames[i] = ianim.nodes[i].frames.size();
		if(numFrames[i] > 0){
			stringToLower(ianim.nodes[i].name);
			c.names[ianim.nodes[i].name] = j;
			c.ids[ianim.nodes[i].boneId] = j;
			j++;
			currentKey[i] = &ianim.nodes[i].frames[0];
		}else{
			anim->numNodes--;
			currentKey[i] = 0;
		}
	}

	for(int j = 0; j < 2; j++)
		for(int i = 0; i < numNodes; i++){
			if(numFrames[i] == 0)
				continue;
			*keyframes = *currentKey[i];
			keyframes->key.prev = prevKey[i];
			prevKey[i] = keyframes;
			keyframes++;
			currentKey[i]++;
			numFrames[i]--;
			totalFrames--;
		}

	for(int j = 0; j < totalFrames; j++){
		int n = -1;
		float minTime = numeric_limits<float>::max();
		for(int i = 0; i < numNodes; i++){
			if(numFrames[i] != 0 && prevKey[i]->key.time < minTime){
				minTime = prevKey[i]->key.time;
				n = i;
			}
		}

		*keyframes = *currentKey[n];
		keyframes->key.prev = prevKey[n];
		prevKey[n] = keyframes;
		keyframes++;
		currentKey[n]++;
		numFrames[n]--;
	}

	delete[] prevKey;
	delete[] currentKey;
	delete[] numFrames;

	return anim;
}

AnimPackage
readIfp(ifstream &ifp)
{
	map<string, Animation*> animpackage;
	IfpAnimPackage anpk;
	readIfpAnimPackage(anpk, ifp);

	for(uint i = 0; i < anpk.anims.size(); i++){
		IfpAnim &ianim = anpk.anims[i];
		Animation *anim = convertIfpAnim(ianim);
		stringToLower(ianim.name);
		animpackage[ianim.name] = anim;
	}
	return animpackage;
}

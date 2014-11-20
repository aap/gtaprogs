#include <renderware.h>
#include "common.h"
#include "car.h"
#include "material.h"
#include "mesh.h"
#include "atomic.h"
#include "refframe.h"
#include "rw.h"

using namespace std;

Car::Car(void)
 : m_clump(0)//, m_txd(0)
{
}

Car::~Car(void)
{
	DECREF(m_chassis_hi);
	DECREF(m_chassis_vlo);
	for(uint i = 0; i < m_extras.size(); i++)
		DECREF(m_extras[i]);
	for(partlistiter it = m_parts.begin(); it != m_parts.end(); it++){
		DECREF(it->second->ok);
		DECREF(it->second->dam);
	}
	DECREF(m_clump);
}

void
Car::load(istream &dffstrm, istream &txdstrm)
{
	rw::Clump *clp = new rw::Clump;
	clp->read(dffstrm);
	for(uint i = 0; i < clp->geometryList.size(); i++)
		clp->geometryList[i].cleanUp();
	m_clump = fromRwClump(clp);
	INCREF(m_clump);
	delete clp;

	rw::TextureDictionary *txd = new rw::TextureDictionary;
	txd->read(txdstrm);
	for(uint i = 0; i < txd->texList.size(); i++){
		if(txd->texList[i].platform == rw::PLATFORM_PS2)
			txd->texList[i].convertFromPS2(0x40);
		if(txd->texList[i].platform == rw::PLATFORM_XBOX)
			txd->texList[i].convertFromXbox();
		if(txd->texList[i].dxtCompression)
			txd->texList[i].decompressDxt();
		txd->texList[i].convertTo32Bit();
	}
	TexDict *texdict = fromRwTxd(txd);
	INCREF(texdict);
	delete txd;

	FORALLATOMICS(it, m_clump)
		(*it)->getMesh()->attachTexDict(texdict);
	DECREF(texdict);
}

void
Car::initHier(const char *suffix)
{
	// Chassis
	m_chassis_hi = m_clump->getAtomic("chassis_hi");
	INCREF(m_chassis_hi);
	m_chassis_vlo = m_clump->getAtomic("chassis_vlo");
	INCREF(m_chassis_vlo);

	// Extras
	char name[32];
	int i = 1;
	Atomic *extra;
	do{
		sprintf(name, "extra%d", i);
		extra = m_clump->getAtomic(name);
		if(extra){
			INCREF(extra);
			m_extras.push_back(extra);
			i++;
		}
	}while(extra);

	const char *partnames[] = {
		"boot", "bonnet", "bump_front", "bump_rear",
		"wing_lf", "wing_rf", "windscreen",
		"door_lf", "door_rf", "door_lr", "door_rr"
	};
	int numParts = sizeof(partnames)/sizeof(*partnames);
	for(int i = 0; i < numParts; i++){
		string name = partnames[i];
		char okname[32], damname[32];
		sprintf(okname, "%s%s_ok", name.c_str(), suffix);
		sprintf(damname, "%s%s_dam", name.c_str(), suffix);
		Atomic *ok, *dam;
		ok = m_clump->getAtomic(okname);
		dam = m_clump->getAtomic(damname);
		if(ok || dam){
			INCREF(ok);
			INCREF(dam);
			Part *part = new Part;
			part->ok = ok;
			part->dam = dam;
			m_parts[name] = part;
		}
	}

	Vec4 prim(60, 255, 0, 255);
	Vec4 sec(255, 0, 175, 255);
	prim /= 255.0f; sec /= 255.0f;
	FORALLATOMICS(it, m_clump){
		Mesh *m = (*it)->getMesh();
		FORALLMATERIALS(it, m){
			Vec4 col = (*it)->color;
			if(col == prim)
				m_primcolors.push_back(*it);
			if(col == sec)
				m_seccolors.push_back(*it);
		}
	}

	// Hide stuff
	for(partlistiter it = m_parts.begin(); it != m_parts.end(); it++)
		m_clump->removeAtomic(it->second->dam);
	for(uint i = 0; i < m_extras.size(); i++)
		m_clump->removeAtomic(m_extras[i]);
	m_clump->removeAtomic(m_chassis_vlo);

//	m_clump->getFrame()->print(cout);
}

void
Car::setRefmap(Texture *refmap)
{
/*
	Material *m;
	for(uint i = 0; i < m_primcolors.size(); i++){
		m = m_primcolors[i];
		if(!m->hasRefmap())
			m->setRef(refmap);
	}
	for(uint i = 0; i < m_seccolors.size(); i++){
		m = m_seccolors[i];
		if(!m->hasRefmap())
			m->setRef(refmap);
	}
*/
}

void
Car::setPrimColor(const Vec4 &col)
{
	for(uint i = 0; i < m_primcolors.size(); i++)
		m_primcolors[i]->color = col;
}

void
Car::setSecColor(const Vec4 &col)
{
	for(uint i = 0; i < m_seccolors.size(); i++)
		m_seccolors[i]->color = col;
}

void
Car::attachWheel(Atomic *atm, float scale)
{
	const char *names[4] = {
		"wheel_rb_dummy", "wheel_lb_dummy",
		"wheel_rf_dummy", "wheel_lf_dummy"
	};
	RefFrame *root = m_clump->getFrame();
	for(int i = 0; i < 4; i++){
		RefFrame *child = root->getChild(names[i]);
		if(child){
			Mat4 &trans = child->getMatrix();
			trans *= Mat4::scale(Vec3(scale, scale, scale));
			Atomic *wheel = atm->clone();
			m_clump->addAtomic(wheel);
			wheel->setFrame(child);
		}
	}
}

// TODO: draw distance
void
Car::render(void)
{
	m_clump->render();
}

Clump*
Car::getClump(void)
{
	return m_clump;
}

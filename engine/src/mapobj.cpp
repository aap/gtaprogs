#include <renderware.h>
#include "common.h"
#include "material.h"
#include "mesh.h"
#include "refframe.h"
#include "atomic.h"
#include "mapobj.h"
#include "rw.h"

using namespace std;

MapObj::MapObj(void)
 : m_clump(0), m_txd(0), m_numLevels(0),
   m_damaged(0), m_isDamaged(false)
{
}

MapObj::~MapObj(void)
{
	DECREF(m_clump);
	DECREF(m_txd);
	DECREF(m_frame);
}

void
MapObj::load(istream &dffstrm, istream &txdstrm)
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
	m_txd = fromRwTxd(txd);
	INCREF(m_txd);
	delete txd;

	FORALLATOMICS(it, m_clump)
		(*it)->getMesh()->attachTexDict(m_txd);

	m_frame = new RefFrame;
	INCREF(m_frame);
}

void
MapObj::initLevels(int numLevels, float *dists)
{
	const char *suffix, *name;
	m_numLevels = 0;
	FORALLATOMICS(it, m_clump){
		if(numLevels == 1){
			m_numLevels = 1;
			m_levels[0].atm = *it;
			m_levels[0].dist = dists[0];
			break;
		}
		if(m_numLevels >= numLevels){
			cerr << "skipping" SP (m_numLevels+1 - numLevels) SP "atomics\n";
			break;
		}
		name = (*it)->getFrame()->name.c_str();
		suffix = name + strlen(name) - 2;
		if(suffix < name)
			continue;
		if((suffix[0] == 'L' || suffix[0] == 'l') && isdigit(suffix[1])){
			int level = suffix[1]-'0';
			m_levels[level].atm = *it;
			m_levels[level].dist = dists[level];
			m_numLevels++;
		}else{
			cerr << "can't handle atomic " << name << endl;
		}
	}

	m_damaged = 0;
	m_isDamaged = false;
	float lastdist = 0.0f;
	for(int i = 0; i < m_numLevels; i++){
		m_levels[i].atm->setFrame(m_frame);
		if(lastdist >= m_levels[i].dist)
			m_damaged = i;
		lastdist = m_levels[i].dist;
	}
}

void
MapObj::setDamage(bool d)
{
	m_isDamaged = d;
}

void
MapObj::render(float dist)
{
	int i = 0;
	if(m_isDamaged && m_damaged)
		i = m_damaged;
	for(; i < m_numLevels; i++){
		if(dist <= m_levels[i].dist){
			m_levels[i].atm->render();
			break;
		}
	}
}

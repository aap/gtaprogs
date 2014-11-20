class MapObj
{
public:
	MapObj(void);
	~MapObj(void);
	void load(std::istream &dffstrm, std::istream &txdstrm);
	void initLevels(int numLevels, float *dists);
	void setDamage(bool d);
	void render(float dist);
private:
	Clump *m_clump;
	TexDict *m_txd;
	RefFrame *m_frame;

	struct Level {
		Atomic *atm;
		float dist;
	};
	Level m_levels[3];
	int m_numLevels;

	int m_damaged;
	bool m_isDamaged;

	MapObj(const MapObj&);
	MapObj &operator=(const MapObj&);
};

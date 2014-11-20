class Car
{
public:
	Car(void);
	~Car(void);
	void load(std::istream &dffstrm, std::istream &txdstrm);
	void initHier(const char *suffix="");
	void setRefmap(Texture *refmap);
	void setPrimColor(const Vec4 &col);
	void setSecColor(const Vec4 &col);
	void attachWheel(Atomic *atm, float scale = 1.0f);
	void render(void);
	Clump *getClump(void);
private:
	Clump *m_clump;

	struct Part {
		Atomic *ok;
		Atomic *dam;
	};
	typedef std::map<std::string, Part*>::iterator partlistiter;
	Atomic *m_chassis_hi, *m_chassis_vlo;
	std::vector<Atomic*> m_extras;
	std::map<std::string, Part*> m_parts;

	std::vector<Material*> m_primcolors;
	std::vector<Material*> m_seccolors;

	Car(const Car&);
	Car &operator=(const Car&);
};

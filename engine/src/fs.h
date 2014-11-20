class FileDir
{
public:
	struct File {
		std::string name;
		std::string path;
		uint start;
		uint length;
	};

	static std::string correctPathCase(const std::string &path);
	void addFromFile(std::istream &dir, std::string path);
	void addFile(File *f);
	void addFile(std::string name, std::string path, uint start, uint length);
	std::istream *getHandle(uint i);
	std::istream *getHandle(std::string name);
	void returnHandle(std::istream *in);

	void dump(void);

private:
	std::deque<File*> m_files;
};

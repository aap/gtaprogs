#include "common.h"
#include "fs.h"

#ifndef _WIN32
	#include <sys/types.h>
	#include <dirent.h>
	#define PSEP_C '/'
	#define PSEP_S "/"
#else
	#define PSEP_C '\\'
	#define PSEP_S "\\"
#endif

using namespace std;

std::string
FileDir::correctPathCase(const string &path)
{
#ifndef _WIN32
	DIR *dir;
	struct dirent *dirent;

	vector<string> subFolders;
	splitString(path, PSEP_C, subFolders);

	string pathSoFar;
	uint i = 0;
	if(path[0] != '/')
		pathSoFar = basepath;
	else{
		pathSoFar = '/';
		i = 1;
	}
	for(; i < subFolders.size(); i++){
		if((dir = opendir(pathSoFar.c_str())) == 0){
			cerr << "couldn't open dir: " << pathSoFar << endl;
			return path;
		}
		while((dirent = readdir(dir)) != 0){
			string fsName = dirent->d_name;
			stringToLower(fsName);
			string tmp = subFolders[i];
			stringToLower(tmp);
			if (tmp == fsName) {
				pathSoFar += PSEP_S;
				pathSoFar += dirent->d_name;
				break;
			}
		}
		closedir(dir);
		if(dirent == 0)
			return path;
	}

	// remove basepath/
	if(path[0] != '/')
		return pathSoFar.substr(basepath.size() + 1);
	// remove first '/' (looks like: //absolute/path/...)
	else
		return pathSoFar.substr(1);
#else
	return path;
#endif
}

void
FileDir::addFromFile(istream &dir, std::string path)
{
	char fourcc[5];
	uint numEntries;

	memset(fourcc, 0, 5);
	dir.read(fourcc, 4);
	if(strcmp(fourcc, "VER2") == 0)
		dir.read((char*)&numEntries, 4);
	else{
		dir.seekg(0, ios::end);
		numEntries = dir.tellg();
		numEntries /= 32;
		dir.seekg(0, ios::beg);
	}
	for(uint i = 0; i < numEntries; i++){
		File *f = new File;
		dir.read((char*)&f->start, 4);
		dir.read((char*)&f->length, 4);
		f->start *= 2048;
		f->length *= 2048;
		char name[25];
		memset(name, 0, 25);
		dir.read(name, 24);
		f->name = name;
		f->path = path;
		addFile(f);
	}
}

void
FileDir::addFile(File *f)
{
	stringToLower(f->name);
	if(m_files.size() > 0){
		string s = f->name;

		int min, max, mid;
		min = 0; max = m_files.size() - 1;

		while(min <= max){
			mid = (min+max) / 2;
			if(m_files[mid]->name == s){
				m_files[mid]->name = f->name;
				m_files[mid]->path = f->path;
				m_files[mid]->start = f->start;
				m_files[mid]->length = f->length;
				delete f;
				return;
			}
			if(m_files[mid]->name > s)
				max = mid - 1;
			else if(m_files[mid]->name < s)
				min = mid + 1;
		}
		m_files.insert(m_files.begin()+min, f);
		return;
	}
	m_files.push_back(f);
}

void
FileDir::addFile(std::string name, std::string path, uint start, uint length)
{
	File *f = new File;
	f->name = name;
	f->path = path;
	f->start = start;
	f->length = length;
	addFile(f);
}

istream*
FileDir::getHandle(uint i)
{
	string path = (basepath+m_files[i]->path).c_str();
	ifstream *f = new ifstream(path.c_str(), ios::binary);
	if(f->fail()){
		delete f;
		return 0;
	}
	f->seekg(m_files[i]->start, ios::beg);
	return f;
}

istream*
FileDir::getHandle(string name)
{
        string s;

        s = name;
        stringToLower(s);

        int min, max, mid;
        min = 0; max = m_files.size() - 1;

        while(min <= max){
                mid = (min+max) / 2;
                if(m_files[mid]->name == s)
                        return getHandle(mid);
                if(m_files[mid]->name > s)
                        max = mid - 1;
                else if(m_files[mid]->name < s)
                        min = mid + 1;
        }
        return 0;
}

void
FileDir::returnHandle(std::istream *in)
{
	((ifstream*)in)->close();
	delete in;
}

void
FileDir::dump(void)
{
	for(uint i = 0; i < m_files.size(); i++)
		cout << m_files[i]->name SP m_files[i]->path << endl;
}

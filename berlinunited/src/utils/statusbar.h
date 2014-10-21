#ifndef STATUSBAR_H_
#define STATUSBAR_H_


#include <sstream>
#include <string>

class Statusbar {
public:
	Statusbar(int height);
	~Statusbar();

	void clear();
	void update(std::stringstream ss);
	void print(int line, std::string str);

private:
	int height;
	int startLine;

	void release();
	void aquire();
};





#endif

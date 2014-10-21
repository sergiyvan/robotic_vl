#ifndef BEEPER_H
#define BEEPER_H

class Beeper {
public:
	virtual ~Beeper() {}

	virtual bool init() { return true; }

	virtual void activateBeepType(int type) {};
};

#endif


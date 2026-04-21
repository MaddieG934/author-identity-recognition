#ifndef PERSON_H
#define PERSON_H

using namespace std;

class Person {
private:
	double pr_repeat;
	double pr_hit;

	double deg_sp;
	double deg_kb;

	int name;

public:
	Person() = default;
	Person(double, double, double, double, int);

	void setSpParams(double, double);
	void displaySpParams();

	void setKbParams(double, double);
	void displayKbParams();

	double getPrRepeat() { return pr_repeat; }
	double getPrMoveOn() { return (1 - pr_repeat); }

	double getPrHit() { return pr_hit; }
	double getPrMiss() { return (1 - pr_hit); }

	double getDegSp() { return deg_sp; }
	double getDegKb() { return deg_kb; }

	void setName(int);
	int getName() { return name; }
};

#endif

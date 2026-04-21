#include <iostream>
#include "person.h"

using namespace std;

Person::Person(double pRepeat, double pHit, double degSp, double degKb, int id)
{
	pr_repeat = pRepeat;
	pr_hit = pHit;

	deg_sp = degSp;
	deg_kb = degKb;

	name = id;
}

void Person::setSpParams(double pRepeat, double degSp)
{
	pr_repeat = pRepeat;
	deg_sp = degSp;
}

void Person::displaySpParams()
{
	cout << "pr_repeat: " << pr_repeat << endl
		<< "pr_moveOn: " << (1 - pr_repeat) << endl
		<< "deg_sp: " << deg_sp << endl;
 }

void Person::setKbParams(double pHit, double degKb)
{
	pr_hit = pHit;
	deg_kb = degKb;
}

void Person::displayKbParams()
{
	cout << "pr_hit: " << pr_hit << endl
		<< "pr_miss: " << (1 - pr_hit) << endl
		<< "deg_kb: " << deg_kb << endl;
}

void Person::setName(int id)
{
	name = id;
}
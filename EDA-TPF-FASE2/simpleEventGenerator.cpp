#include "simpleEventGenerator.h"

using namespace std;

simpleEventGenerator::simpleEventGenerator() :pos(0) {}

genericEvent * simpleEventGenerator::
getEvent(void)
{
	genericEvent * ret = nullptr;
	switch (evento)
	{
	case EventA:
		ret = new cEventA;
		break;
	case EventB:
		ret = new cEventB;
		break;
	case EventC:
		ret = new cEventC;
		break;
	case EventD:
		ret = new cEventD;
		break;
	case EventQuit:
		ret = new cEventQuit;
		break;
	}
	pos++;
	return ret;
}

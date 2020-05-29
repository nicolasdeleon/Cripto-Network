#pragma once
#include <array>

#include "eventHandling.h"

enum implEvent : eventTypes { EventA, EventB, EventC, EventD, EventQuit };

class cEventA : public genericEvent
{
public:
	eventTypes getType(void) { return EventA; }
};

class cEventB : public genericEvent
{
public:
	eventTypes getType(void) { return EventB; }
};

class cEventC : public genericEvent
{
public:
	eventTypes getType(void) { return EventC; }
};

class cEventD : public genericEvent
{
public:
	eventTypes getType(void) { return EventD; }
};

class cEventQuit : public genericEvent
{
public:
	eventTypes getType(void) { return EventQuit; }
};


class simpleEventGenerator : public eventGenerator
{

	public:
	simpleEventGenerator();
	genericEvent * getEvent(void);
	
	private:

	unsigned int pos;
	eventTypes evento;
	
};

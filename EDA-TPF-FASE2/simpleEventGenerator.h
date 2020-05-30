#pragma once

#include <queue>
#include <allegro5/allegro.h>
#include "eventHandling.h"

using namespace std;

enum implEvent : eventTypes { EventDraw, EventCreateNodeScreen, EventBack, EventCreateNode, EventQuit};

class cEventDraw : public genericEvent
{
public:
	eventTypes getType(void) { return EventDraw; }
};

class cEventCreateNodeScreen : public genericEvent
{
public:
	eventTypes getType(void) { return EventCreateNodeScreen; }
};

class cEventQuit : public genericEvent
{
public:
	eventTypes getType(void) { return EventQuit; }
};

class cEventBack : public genericEvent
{
public:
	eventTypes getType(void) { return EventBack; }
};

class cEventCreateNode : public genericEvent
{
public:
	cEventCreateNode(const char* aliasName, const char* ipAdress, const char* clientPort) :
		alias(aliasName), ip(ipAdress), port(clientPort) {};
	eventTypes getType(void) { return EventCreateNode; }
	const char* alias, *ip, *port;
};


class interfaseEventGenerator : public eventGenerator
{

	public:
	interfaseEventGenerator();
	~interfaseEventGenerator();
	genericEvent * getEvent(void);
	void printMainMenu(void);
	void printMakingNode(void);
	void printManageConnections(void);
	
	
	private:

	
	queue <genericEvent*> guiEvents;
	ALLEGRO_EVENT_QUEUE* alEventQueue;

	ALLEGRO_DISPLAY* display;

	bool imguiInit();
};


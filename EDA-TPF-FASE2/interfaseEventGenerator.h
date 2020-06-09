#pragma once

#include <queue>
#include <allegro5/allegro.h>
#include "eventHandling.h"
#include "Simulation.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>


using namespace std;
namespace fs = boost::filesystem;

enum implEvent : eventTypes { E_Draw, E_CreateNode, E_Back, E_MngNodeCnx, E_MakeTsx, E_Quit};

class cEventDraw : public genericEvent
{
public:
	eventTypes getType(void) { return E_Draw; }
};

class cEventCreateNodeScreen : public genericEvent
{
public:
	eventTypes getType(void) { return E_CreateNode; }
};

class cEventQuit : public genericEvent
{
public:
	eventTypes getType(void) { return E_Quit; }
};

class cEventBack : public genericEvent
{
public:
	eventTypes getType(void) { return E_Back; }
};

class cEventManageConnections : public genericEvent
{
public:
	eventTypes getType(void) { return E_MngNodeCnx; }
};

class cEventMakeTsx : public genericEvent
{
public:
	eventTypes getType(void) { return E_MakeTsx; }
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
	void printMakeTsx(void);
	void printChooseMode(void);
	bool genesisGo();
	void linkSimulation(Simulation* sim) {
		mySim = sim;
		currentNodes = mySim->getNodes();
	}
	string getFilename();
	
	private:

	string filename;
	bool failed;
	bool print_SelectJsons(vector<string>& nombres);
	vector<string> lookForJsonFiles(const char* directoryName);
	string directory;
	vector<string> jsonPaths;
	vector<string> extract_keys(std::map<std::string, boost::asio::ip::tcp::socket*> const& input_map);
	queue <genericEvent*> guiEvents;
	ALLEGRO_EVENT_QUEUE* alEventQueue;
	ALLEGRO_DISPLAY* display;
	vector<GenericNode*> currentNodes;
	vector<string> connecting_to;
	Simulation* mySim;
	bool imguiInit();

};


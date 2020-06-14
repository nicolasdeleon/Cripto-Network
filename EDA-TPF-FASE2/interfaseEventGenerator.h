#pragma once

#include <queue>
#include <allegro5/allegro.h>
#include "eventHandling.h"
#include "Simulation.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "blockchainHandler.h"

#define MY_NODE_FULL 0

using namespace std;
namespace fs = boost::filesystem;

enum implEvent : eventTypes { E_Draw, E_CreateNode, E_Back, E_MngNodeCnx, E_MakeTsx, E_Info, E_Quit};
enum class POPS { ValidateMerkle, Merkle, Info, Failed, Default };

typedef struct {
	string blockId;
	string previousBlockId;
	string NTransactions;
	string BlockNumber;
	string nonce;
	bool show = false;
} BlockInfo;

typedef struct {
	string tree;
	bool show = false;
} BlockTree;

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

class cEventInfo : public genericEvent
{
public:
	eventTypes getType(void) { return E_Info; }
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
	void printInfoWindow(void);
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
	void pushBackEvent();
	void pushCnxEvent();
	
	private:
	void showBlockInfo(int index);
	void printTree(vector<vector<string>> Tree);
	string filename;
	bool failed;
	bool print_SelectJsons(vector<string>& nombres);
	vector<string> lookForJsonFiles(const char* directoryName);
	string directory;
	vector<string> jsonPaths;
	BlockTree displayTree;
	vector<string> extract_keys(std::map<std::string, boost::asio::ip::tcp::socket*> const& input_map);
	queue <genericEvent*> guiEvents;
	ALLEGRO_EVENT_QUEUE* alEventQueue;
	ALLEGRO_DISPLAY* display;
	vector<GenericNode*> currentNodes;
	vector<string> connecting_to;
	Simulation* mySim;
	BlockInfo displayInfo;
	blockchainHandler blockchainHandler;
	bool notParsed;
	bool imguiInit();

};


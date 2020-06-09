#include <iostream>
#include "eventHandling.h"
#include "genericFSM.h"
#include "interfaseEventGenerator.h"
#include "Simulation.h"
#include <stdlib.h>


enum implStates: stateTypes {MainMenu, CreatingNode, ManageConnections, MakeTsx, WelcomeScreen, workGenesis, dummyState};

using namespace std;
class FSMImplementation : public genericFSM
{
	
	
	private:
		
	#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSMImplementation::x)) //casteo a funcion, por visual
		const fsmCell fsmTable[6][5] = {
			//  E_Draw							    E_CreateNode                       E_Back                             E_MngNodeCnx							E_MakeTsx													
			{  	{MainMenu,TX(printMainMenu)},		{CreatingNode,TX(dummyfunc)},	   {MainMenu,TX(dummyfunc)},	      {ManageConnections,TX(dummyfunc)},	{MakeTsx,TX(dummyfunc)}},  //MainMenu
			{	{CreatingNode,TX(printMakingNode)},	{ManageConnections,TX(dummyfunc)}, {MainMenu,TX(dummyfunc)},		  {CreatingNode,TX(dummyfunc)},		    {CreatingNode,TX(dummyfunc)}},  //CreatingNode
			{	{ManageConnections,TX(printManageCnx)},	{dummyState,TX(dummyfunc)},	   {MainMenu,TX(dummyfunc)},		  {ManageConnections,TX(dummyfunc)},    {ManageConnections,TX(dummyfunc)}},  //ManageConnections 
			{	{MakeTsx,TX(printMakeTsx)},	        {MainMenu,TX(dummyfunc)},	       {MainMenu,TX(dummyfunc)},		  {MakeTsx,TX(dummyfunc)},		        {MakeTsx,TX(dummyfunc)}},	  //MakeTsx
			{	{WelcomeScreen,TX(printWelcomeScreen)},	{WelcomeScreen,TX(dummyfunc)}, {MainMenu,TX(setupApendix)},		  {workGenesis,TX(setupGenesis)},			{WelcomeScreen,TX(dummyfunc)}},	  //welcomeScreen
			{   {workGenesis,TX(checkifReady)},		{workGenesis,TX(dummyfunc)},	   {MainMenu,TX(dummyfunc)},		  {MainMenu,TX(dummyfunc)},	    {workGenesis,TX(dummyfunc)}}	  //welcomeScreen
			};
	
	//The action routines for the FSM
	//These actions should not generate fsmEvents
	
	void dummyfunc(genericEvent * ev)
	{
		return;
	}

	void printMainMenu(genericEvent* ev) {
		guiEvGen->printMainMenu();
		return;
	}

	void printMakingNode(genericEvent* ev) {
		guiEvGen->printMakingNode();
		return;
	}

	void printManageCnx(genericEvent* ev) {
		guiEvGen->printManageConnections();
		return;
	}

	void printMakeTsx(genericEvent* ev) {
		guiEvGen->printMakeTsx();
		return;
	}

	void printWelcomeScreen(genericEvent* ev) {
		guiEvGen->printChooseMode();
		return;
	}

	void setupApendix(genericEvent* ev) {
		cout << "setupAp" << endl;
		return;
	}

	void setupGenesis(genericEvent* ev) {

		cout << "setupGen" << endl;
		json creador;

		string str = guiEvGen->getFilename();

		//Abro el archivo y lo asigno a mi variable miembro blocks (de tipo json)
		ifstream blocks_file(str, ifstream::binary);

		blocks_file >> creador;

		string creador_str = creador.dump();
		if (!creador.empty())
		{

			for (string node : creador["full-nodes"]) {

				sim->addNode("127.0.0.1", stoi(node), NodeType::FULL);
			}
			for (string node2 : creador["spv"]) {
				sim->spvGenNodes.push_back(stoi(node2));
			}

			vector<string> addresses = creador["full-nodes"];
			sim->giveAddress2Nodes(addresses);

		}

		// terminar las configs con un startNodes()
		sim->startNodes();
		return;
	}

	void checkifReady(genericEvent* ev) {
		if (sim->areFullReady() && sim->spvGenNodes.size()) {
			sim->connectSPV();
		}
		else if (!(sim->spvGenNodes.size())) {
			guiEvGen->pushBackEvent();
		}	
	}

	interfaseEventGenerator* guiEvGen;
	Simulation* sim;

	public:
	FSMImplementation(): genericFSM(&fsmTable[0][0],5,6, WelcomeScreen){}
	void referenceGuiEvGen(interfaseEventGenerator * guiEvGenerator) {
		guiEvGen = guiEvGenerator;
	}
	void referenceNodes(Simulation* simulation) {
		sim = simulation;
	}
};


int main(int argc, char** argv) 
{
	Simulation sim;
	FSMImplementation fsm;
	interfaseEventGenerator guiEvGen;	//generador de UN tipo de eventos
	mainEventGenerator eventGen;	//generador de eventos de TODO el programa
	
	//netwEventGenerator netwEvGen;
	fsm.referenceGuiEvGen(&guiEvGen);
	fsm.referenceNodes(&sim);
	eventGen.attach(&guiEvGen);	//registro fuente de eventos


	srand(time(NULL));

	guiEvGen.linkSimulation(&sim);
	/* Esto serian configuraciones pre iniciar el programa */

	
	bool quit = false;
	
	do
	{
		genericEvent * ev = eventGen.getNextEvent();

		if (ev != nullptr) 
		{
			if (ev->getType() == E_Quit)
			{
				quit = true;
			}
			else {
				sim.doNodePolls();
				fsm.cycle(ev);
			}
			delete ev;
		}
	} 
	while (!quit);
	
	return 0;
}

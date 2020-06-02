#include <iostream>
#include "eventHandling.h"
#include "genericFSM.h"
#include "interfaseEventGenerator.h"
#include "Simulation.h"

enum implStates: stateTypes {MainMenu, CreatingNode, ManageConnections, dummyState};

using namespace std;
class FSMImplementation : public genericFSM
{
	
	private:
		
	#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSMImplementation::x)) //casteo a funcion, por visual
		const fsmCell fsmTable[4][4] = {
			//  E_Draw							    E_CreateNode                       E_Back                             E_MngNodeCnx 
			{  	{MainMenu,TX(printMainMenu)},		{CreatingNode,TX(dummyfunc)},	   {dummyState,TX(dummyfunc)},	      {ManageConnections,TX(dummyfunc)}},		//MainMenu
			{	{CreatingNode,TX(printMakingNode)},	{ManageConnections,TX(dummyfunc)}, {MainMenu,TX(dummyfunc)},		  {CreatingNode,TX(dummyfunc)}},			//CreatingNode
			{	{ManageConnections,TX(printManageCnx)},	{dummyState,TX(dummyfunc)},		   {MainMenu,TX(dummyfunc)},	  {ManageConnections,TX(dummyfunc)}},		//ManageConnections 
			{	{dummyState,TX(dummyfunc)},	        {MainMenu,TX(dummyfunc)},	       {CreatingNode,TX(dummyfunc)},	  {dummyState,TX(dummyfunc)}}				//dummyState
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

	interfaseEventGenerator* guiEvGen;
	Simulation* sim;

	public:
	FSMImplementation(): genericFSM(&fsmTable[0][0],4,4,MainMenu){}
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
	/* Esto serian configuraciones cuando se cargan todos los nodos */
	sim.addNode("127.0.0.1", 8080, NodeType::SPV);
	sim.addNode("127.0.0.1", 80, NodeType::SPV);
	sim.createConnection("127.0.0.1", 80, "127.0.0.1", 8080);
	// terminar las configs con un startNodes()
	sim.startNodes();
	guiEvGen.linkSimulation(&sim); // kjjjjj pero mirá lo que es esta turbiedad de código, para tener comunicación entre mi set de nodos y la gui :P
	/* Esto serian configuraciones pre iniciar el programa */
	sim.sendMessageFromNode2Node("127.0.0.1", 8080, "127.0.0.1", 80, MessageIds::TRANSACTION);
	bool quit = false;
	do
	{

		genericEvent * ev;
		ev = eventGen.getNextEvent();
		sim.doNodePolls();
		if (ev != nullptr) 
		{
			if (ev->getType() == E_Quit)
			{
				quit = true;
			}
			else
				fsm.cycle(ev);
			delete ev;
		}
	} 
	while (!quit);
		
	//system("pause");
	return 0;
}




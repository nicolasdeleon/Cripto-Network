#include <iostream>
#include "eventHandling.h"
#include "genericFSM.h"
#include "simpleEventGenerator.h"
#include "Simulation.h"

enum implStates: stateTypes {MainMenu, CreatingNode, ManageConnections, dummyState};

using namespace std;
class FSMImplementation : public genericFSM
{
	
	
	private:
		
	#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSMImplementation::x)) //casteo a funcion, por visual
		const fsmCell fsmTable[4][4] = {
			//  EventDraw							EventCreateNodeScreen              EventBack                            EventCreateNode					   
			{  	{MainMenu,TX(printMainMenu)},		{CreatingNode,TX(dummyfunc)},	   {dummyState,TX(dummyfunc)},	        {dummyState,TX(dummyfunc)}},			//MainMenu
			{	{CreatingNode,TX(printMakingNode)},	{ManageConnections,TX(dummyfunc)}, {MainMenu,TX(dummyfunc)},			{MainMenu,TX(createNode)}},				//CreatingNode
			{	{ManageConnections,TX(dummyfunc)},	{dummyState,TX(dummyfunc)},		   {MainMenu,TX(dummyfunc)},			{CreatingNode,TX(dummyfunc)}},			//ManageConnections 
			{	{dummyState,TX(dummyfunc)},	        {MainMenu,TX(dummyfunc)},	       {CreatingNode,TX(dummyfunc)},		{ManageConnections,TX(dummyfunc)}}		//dummyState
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

	void createNode(genericEvent* evBase) {
		cEventCreateNode * ev = static_cast<cEventCreateNode*>(evBase);
		cout << "CreateNode: alias = " << ev->alias << ", ip = "<< ev->ip
			<< ", portNumber = " << ev->port << endl;
		return;
	}

	interfaseEventGenerator* guiEvGen;

	public:
	FSMImplementation(): genericFSM(&fsmTable[0][0],4,4,MainMenu){}
	void referenceGuiEvGen(interfaseEventGenerator * guiEvGenerator) {
		guiEvGen = guiEvGenerator;
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
	eventGen.attach(&guiEvGen);	//registro fuente de eventos
	/* Esto serian configuraciones cuando se cargan todos los nodos */
	sim.addNode(8080);
	sim.addNode(80);
	// terminar las configs con un startNodes()
	sim.startNodes();
	/* Esto serian configuraciones pre iniciar el programa */

	bool quit = false;
	do
	{

		genericEvent * ev;
		ev = eventGen.getNextEvent();
		if (ev != nullptr) 
		{
			sim.doNodePolls();
			if (ev->getType() == EventQuit)
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




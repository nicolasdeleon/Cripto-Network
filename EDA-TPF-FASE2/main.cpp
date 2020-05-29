#include <iostream>
#include "eventHandling.h"
#include "genericFSM.h"
#include "simpleEventGenerator.h"

enum implStates: stateTypes {State0, State1, State2, State3};

using namespace std;
class FSMImplementation : public genericFSM
{
	
	
	private:
		
	#define TX(x) (static_cast<void (genericFSM::* )(genericEvent *)>(&FSMImplementation::x)) //casteo a funcion, por visual
	const fsmCell fsmTable[4][4] =  {
		//       EventA                 EventB                  EventC                  EventD
		{  	{State0,TX(prueba1)},		{State1,TX(prueba2)},		{State2,TX(prueba3)},		{State3,TX(prueba4)}},   //State0
		{	{State1,TX(prueba1)},		{State2,TX(prueba2)},		{State3,TX(prueba3)},		{State0,TX(prueba4)}},   //State1
		{	{State2,TX(prueba1)},		{State3,TX(prueba2)},		{State0,TX(prueba3)},		{State1,TX(prueba1)}},   //State2
		{	{State3,TX(prueba1)},		{State0,TX(prueba2)},		{State1,TX(prueba3)},		{State2,TX(prueba4)}}    //State3 
		};
	
	//The action routines for the FSM
	//These actions should not generate fsmEvents
	
	void prueba1(genericEvent * ev)
	{
		cout << "prueba 1" <<endl;
		return;
	}
	void prueba2(genericEvent * ev)
	{
		cout << "prueba 2" <<endl;
		return;
	}
	void prueba3(genericEvent * ev)
	{
		cout << "prueba 3" <<endl;
		return;
	}
	void prueba4(genericEvent * ev)
	{
		cout << "prueba 4" <<endl;
		return;
	}
	
	public:
	FSMImplementation(): genericFSM(&fsmTable[0][0],4,4,State0){}
};


int main(int argc, char** argv) 
{
	FSMImplementation fsm;
	simpleEventGenerator s;	//generador de UN tipo de eventos
	mainEventGenerator eventGen;	//generador de eventos de TODO el programa

	//guiEventGenerator guiEvGen;
	//netwEventGenerator netwEvGen;

	eventGen.attach(&s);	//registro fuente de eventos
	
	bool quit = false;
	do
	{
		genericEvent * ev;
		ev = eventGen.getNextEvent();
		if (ev != nullptr) 
		{
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
		
	system("pause");
	return 0;
}




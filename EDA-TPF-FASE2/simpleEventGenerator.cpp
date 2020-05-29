#include "simpleEventGenerator.h"
#include <iostream>


#define SIZE_SCREEN_X 500
#define SIZE_SCREEN_Y 500

using namespace std;

simpleEventGenerator::simpleEventGenerator() {
	display = nullptr;
	alEventQueue = nullptr;
	if (al_init()) {
		display = al_create_display(SIZE_SCREEN_X, SIZE_SCREEN_Y);
		alEventQueue = al_create_event_queue();
		if (display && al_install_mouse() && al_install_keyboard()) {
			al_register_event_source(alEventQueue, al_get_display_event_source(display));
			al_register_event_source(alEventQueue, al_get_mouse_event_source());
			al_register_event_source(alEventQueue, al_get_keyboard_event_source());
		}
	}
}

simpleEventGenerator::~simpleEventGenerator()
{
	al_uninstall_keyboard();
	al_uninstall_mouse();
	if (alEventQueue)
		al_destroy_event_queue(alEventQueue);
	if (display)
		al_destroy_display(display);
}

genericEvent * simpleEventGenerator::
getEvent(void)
{

	genericEvent * ret = nullptr;
	ALLEGRO_EVENT ev;
	if (al_get_next_event(alEventQueue, &ev)) {
		switch (ev.type)
		{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			cout << "I'm out!" << endl;
			ret = new cEventQuit;
			break;
		default:
			ret = new cEventA;
		}
	}
	return ret;
}

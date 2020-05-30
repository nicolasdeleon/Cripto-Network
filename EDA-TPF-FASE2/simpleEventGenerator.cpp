#include "simpleEventGenerator.h"

#include <iostream>

#include "imgui_files/imgui.h"
#include "imgui_files/imgui_impl_allegro5.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>


#define SIZE_SCREEN_X 600
#define SIZE_SCREEN_Y 500


interfaseEventGenerator::interfaseEventGenerator() {
	display = nullptr;
	alEventQueue = nullptr;
	if (al_init()) {
		display = al_create_display(SIZE_SCREEN_X, SIZE_SCREEN_Y);
		alEventQueue = al_create_event_queue();
		if (display && al_install_mouse() && al_install_keyboard()) {
			al_register_event_source(alEventQueue, al_get_display_event_source(display));
			al_register_event_source(alEventQueue, al_get_mouse_event_source());
			al_register_event_source(alEventQueue, al_get_keyboard_event_source());
			al_init_image_addon();
			al_init_primitives_addon();
			imguiInit();
		}

	}
}

bool interfaseEventGenerator::imguiInit(void)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls


	al_set_window_position(display, 500, 100); //posicion del menu

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplAllegro5_Init(display);


	return true;
}

interfaseEventGenerator::~interfaseEventGenerator()
{
	al_uninstall_keyboard();
	al_uninstall_mouse();
	if (alEventQueue)
		al_destroy_event_queue(alEventQueue);
	if (display)
		al_destroy_display(display);
}

genericEvent * interfaseEventGenerator::
getEvent(void)
{

	genericEvent * ret = nullptr;
	ALLEGRO_EVENT ev;
	if (al_get_next_event(alEventQueue, &ev)) {
		
		ImGui_ImplAllegro5_ProcessEvent(&ev);
		switch (ev.type)
		{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			cout << "I'm out!" << endl;
			ret = new cEventQuit;
			break;			
		
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			al_acknowledge_resize(display);
			ImGui_ImplAllegro5_CreateDeviceObjects();
			break;
		}
		
	}
	else if (!guiEvents.empty()) {
		ret = guiEvents.front();
		guiEvents.pop();
	}
	else {
		
		ret = new cEventDraw;
	}
	return ret;
}

void interfaseEventGenerator::printMainMenu(void) {
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("Welcome to the EDAcoin", 0, 0);

	//bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);

	if (ImGui::Button("Create a new node")) {
		guiEvents.push(new cEventCreateNodeScreen);
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}

void interfaseEventGenerator::printMakingNode(void) {
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("Welcome to the EDAcoin", 0, 0);

	static char alias[25];
	ImGui::InputText("Node alias", alias, sizeof(char) * 25);

	static char ip[25];
	ImGui::InputText("Node ip", ip, sizeof(char) * 25);

	static char port[25];
	ImGui::InputText("Node client port", port, sizeof(char) * 25);

	if (ImGui::Button("Cancel")) {
		ip[0] = '\0';
		port[0] = '\0';
		alias[0] = '\0';
		guiEvents.push(new cEventBack);
	}

	ImGui::SameLine();

	if (ImGui::Button("Create Node")) {
		guiEvents.push(new cEventCreateNode(alias, ip, port));
		ip[0] = '\0';
		port[0] = '\0';
		alias[0] = '\0';
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}

void interfaseEventGenerator::printManageConnections(void) {
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("Welcome to the EDAcoin", 0, 0);

	static char ip[25];
	ImGui::InputText("Node ip", ip, sizeof(char) * 25);

	static char port[25];
	ImGui::InputText("Node client port", port, sizeof(char) * 25);

	if (ImGui::Button("Cancel")) {
		ip[0] = '\0';
		port[0] = '\0';
		guiEvents.push(new cEventBack);
	}

	ImGui::SameLine();

	if (ImGui::Button("Create new connection")) {
		//crear popup para crear nueva conexión y pushear un evento que cree una conexión
	}

	if (ImGui::Button("Delete connection")) {
		//pushear un evento que elimine la conexión
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}




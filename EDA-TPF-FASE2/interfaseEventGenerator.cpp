#include "interfaseEventGenerator.h"

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

	ImGui::Begin("EDAcoin", 0, 0);

	//bool show_demo_window = true;
	//ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Text("Current Local Nodes: ");

	ImGui::BeginChild("nodes", ImVec2(300, 400), true, ImGuiWindowFlags_None);

	for (int i = 0; i < currentNodes.size(); i++)
	{
		ImGui::BulletText(currentNodes[i]->getAddress().c_str());
	}
	
	ImGui::EndChild();

	if (ImGui::Button("Manage Nodes")) {
		guiEvents.push(new cEventCreateNodeScreen);
	}

	ImGui::SameLine();

	if (ImGui::Button("Manage Node Connections")) {
		guiEvents.push(new cEventManageConnections);
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

	ImGui::Begin("EDAcoin", 0, 0);

	ImGui::BeginChild("Node List", ImVec2(300, 400), true, ImGuiWindowFlags_None);
	
	static int checked = -1;

	for (int i = 0; i < currentNodes.size(); i++)
	{
		ImGui::RadioButton(currentNodes[i]->getAddress().c_str(), &checked, i);
	}

	ImGui::EndChild();



	if (ImGui::Button("Create New Node"))
		ImGui::OpenPopup("New Node");

	if (ImGui::BeginPopupModal("New Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		static char alias[25];
		//ImGui::InputText("Node alias", alias, sizeof(char) * 25);

		static char ip[25];
		ImGui::InputText("Node ip", ip, sizeof(char) * 25, ImGuiInputTextFlags_CharsDecimal);

		static int port;
		ImGui::InputInt("Node port", &port);

		static int nodeTypecheck = -1;
		
		ImGui::RadioButton("FULL", &nodeTypecheck, 0);
		ImGui::RadioButton("SPV", &nodeTypecheck, 1);

		if (ImGui::Button("OK", ImVec2(120, 0)) && nodeTypecheck) {
			NodeType nodeType;

			switch (nodeTypecheck) {
			case 0:
				nodeType = NodeType::FULL;
				break;
			case 1:
				nodeType = NodeType::SPV;
			}

			mySim->addNode(ip, port, nodeType);
			currentNodes = mySim->getNodes();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::Button("Delete Node") && checked != -1)
		ImGui::OpenPopup("Del Node");

	if (ImGui::BeginPopupModal("Delete Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure? a node deletion is permanent!\n");
		if (ImGui::Button("Yes", ImVec2(120, 0))) {
			mySim->deleteNode(currentNodes[checked]->getIP(), currentNodes[checked]->getPort());
			currentNodes = mySim->getNodes();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::Button("Home Menu")) {
		guiEvents.push(new cEventBack);
	}



	ImGui::SameLine();

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

	ImGui::Begin("EDAcoin", 0, 0);

	static int checked = -1;

	ImGui::BeginChild("Curr Nodes", ImVec2(300, 400), true, ImGuiWindowFlags_None);

	for (int i = 0; i < currentNodes.size(); i++)
	{
		ImGui::RadioButton(currentNodes[i]->getAddress().c_str(), &checked, i);
	}

	ImGui::EndChild();

	if (ImGui::Button("Cancel")) {
		guiEvents.push(new cEventBack);
	}	

	if (ImGui::Button("Create New Connection") && checked != -1)
		ImGui::OpenPopup("New Connection");

	if (ImGui::BeginPopupModal("New Connection", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		static char ip[25];
		ImGui::InputText("Node ip", ip, sizeof(char) * 25, ImGuiInputTextFlags_CharsDecimal);

		static int port;
		ImGui::InputInt("Node port", &port);


		if (ImGui::Button("OK", ImVec2(120, 0))) { 
			
			mySim->createConnection(
				currentNodes[checked]->getIP(),
				currentNodes[checked]->getPort(),
				ip,
				port
				);
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	
	static vector<string> keysVector;

	if (ImGui::Button("Delete connection") && checked != -1) {
		ImGui::OpenPopup("Delete Connection");
		map<string, boost::asio::ip::tcp::socket*> connectionsMap = currentNodes[checked]->getConnections();
		for (map<string, boost::asio::ip::tcp::socket*>::iterator it = connectionsMap.begin(); it != connectionsMap.end(); ++it) {
			keysVector.push_back(it->first);
		}
	}


	if (ImGui::BeginPopupModal("Delete Connection", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		static int checkedCnx = -1;

		for (int i = 0; i < keysVector.size(); i++)
		{
			ImGui::RadioButton(keysVector[i].c_str(), &checkedCnx, i);
		}

		if (ImGui::Button("OK", ImVec2(120, 0)) && checked) {
			mySim->deleteConnection(currentNodes[checked]->getAddress(), keysVector[checkedCnx]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::End();

	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}




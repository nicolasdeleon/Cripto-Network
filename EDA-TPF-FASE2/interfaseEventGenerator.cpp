#include <iostream>

#include "interfaseEventGenerator.h"

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

	ImGui::SameLine();

	if (ImGui::Button("Make new transacion")) {
		guiEvents.push(new cEventMakeTsx);
	}

	ImGui::End();

	//Rendering
	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}

void interfaseEventGenerator::printMakingNode(void) {
	currentNodes = mySim->getNodes();
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("EDAcoin", 0, 0);

	ImGui::BeginChild(".json files in current folder", ImVec2(300, 400), true, ImGuiWindowFlags_None);
	
	static int checked = -1;

	for (int i = 0; i < currentNodes.size(); i++)
	{
		ImGui::RadioButton(currentNodes[i]->getAddress().c_str(), &checked, i);
	}

	ImGui::EndChild();

	if (ImGui::Button("Append"))
		ImGui::OpenPopup("Append");

	if (ImGui::BeginPopupModal("Append", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		static int port;
		ImGui::InputInt("Node port", &port);

		
		static int NType = 0;

		ImGui::RadioButton("Full Node", &NType, 0);
		ImGui::RadioButton("SPV Node", &NType, 1);

		if (ImGui::Button("Add connection"))
			ImGui::OpenPopup("Add connection");

		if (ImGui::BeginPopupModal("Add connection", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Please select the connection node's IP and port:\n\n");
			ImGui::Separator();

			static char ip[25];
			ImGui::InputText("Node ip", ip, sizeof(char) * 25, ImGuiInputTextFlags_CharsDecimal);

			static int port;
			ImGui::InputInt("Node port", &port);

			if (ImGui::Button("OK", ImVec2(120, 0))) {
				string address = ip;
				address += ":";
				address += to_string(port);
				connecting_to.push_back(address);
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}

		for (string address : connecting_to) {
			ImGui::Text(address.c_str());
		}
		
		if (ImGui::Button("OK", ImVec2(120, 0))) {
			if (!(port % 2)) {
				string ip = "127.0.0.1";
				unsigned int port_ui = static_cast <unsigned int>(port);
				cout << "APPENDING" << endl;
				mySim->appendNode(ip, port_ui, static_cast <NodeType>(NType), connecting_to);
				currentNodes = mySim->getNodes();
				ImGui::CloseCurrentPopup();
				connecting_to.clear();
			}
			else {
				port++;
			}
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup();
			connecting_to.clear();
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Delete Node") && checked != -1)
		ImGui::OpenPopup("Del Node");

	if (ImGui::BeginPopupModal("Del Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
	currentNodes = mySim->getNodes();
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
			currentNodes = mySim->getNodes();
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	
	

	if (ImGui::Button("Delete connection") && checked != -1) {
		ImGui::OpenPopup("Delete Connection");
	}


	if (ImGui::BeginPopupModal("Delete Connection", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		vector <string> keysVector = extract_keys(currentNodes[checked]->getConnections());

		static int checkedCnx = -1;

		for (int i = 0; i < keysVector.size(); i++)
		{
			ImGui::RadioButton(keysVector[i].c_str(), &checkedCnx, i);
		}

		if (ImGui::Button("OK", ImVec2(120, 0)) && checked) {
			mySim->deleteConnection(currentNodes[checked]->getAddress(), keysVector[checkedCnx]);
			currentNodes = mySim->getNodes();
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

void interfaseEventGenerator::printMakeTsx(void) {
	currentNodes.clear();
	currentNodes = mySim->getNodes();
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
	

	if (ImGui::Button("Make transaction") && checked != -1) {
		ImGui::OpenPopup("Tsx");
	}


	if (ImGui::BeginPopupModal("Tsx", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		vector <string> keysVector = extract_keys(currentNodes[checked]->getConnections());

		static int checkedCnx = -1;

		for (int i = 0; i < keysVector.size(); i++)
		{
			ImGui::RadioButton(keysVector[i].c_str(), &checkedCnx, i);
		}

		ImGui::Text("Please select the size of the transaction: ");
		static int amount;
		ImGui::InputInt("EDA$", &amount);

		if (ImGui::Button("OK", ImVec2(120, 0)) && checkedCnx != -1) {
			
			mySim->sendTransaction(currentNodes[checked]->getAddress(), keysVector[checkedCnx], amount);
			ImGui::CloseCurrentPopup();
			checkedCnx = -1;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup(); 
			checkedCnx = -1;
		}
		ImGui::EndPopup();
	}

	ImGui::End();

	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));


	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());


	al_flip_display();
}

void interfaseEventGenerator::printChooseMode(void) {
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();


	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("EDAcoin", 0, 0);

	if (ImGui::Button("Modo Genesis")) {
		ImGui::OpenPopup("PickupJSON");
	}
	
	if (ImGui::Button("Modo appendix")) {
		guiEvents.push(new cEventBack);
	}

	if (ImGui::BeginPopupModal("PickupJSON", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{

		static char path[MAX_PATH];
		ImGui::InputText("Directorio", path, sizeof(char) * MAX_PATH);


		if (string(path) != "") {

			directory = path;
			jsonPaths = lookForJsonFiles(path);
			if (jsonPaths.size() > 0 && print_SelectJsons(jsonPaths)) {}
		}

		//entra aca solo si hubo un error en el parseo para mostrarlo en forma de pop-up
		if (failed == true)
		{
			ImGui::OpenPopup("Failed");

			if (ImGui::BeginPopupModal("Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Separator();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					failed = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();			
		}
		ImGui::EndPopup();
	}

	ImGui::End();

	ImGui::Render();

	al_clear_to_color(al_map_rgb(211, 211, 211));

	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	al_flip_display();
}

vector<string> interfaseEventGenerator::lookForJsonFiles(const char* directoryName)
{

	vector<string>filenames;

	if (string(directoryName).size() > 2) {

		fs::path bPath(directoryName);

		if (exists(bPath) && is_directory(bPath))
		{
			for (fs::directory_iterator iterator(bPath); iterator != fs::directory_iterator(); iterator++)
			{
				if ((iterator->path().extension().string() == ".json"))
				{
					filenames.push_back(iterator->path().filename().string());
				}
			}
		}

	}

	return filenames;
}


bool interfaseEventGenerator::print_SelectJsons(vector<string>& nombres)
{
	bool eventHappened;

	static int checked = -1;
	ImGui::BeginChild(".json files in current folder", ImVec2(300, 400), true, ImGuiWindowFlags_None);

	for (int i = 0; i < nombres.size(); i++)
	{
		ImGui::RadioButton(nombres[i].c_str(), &checked, i);
	}

	ImGui::EndChild();


	if (ImGui::Button("Seleccionar") && checked != -1)
	{
		for (int i = 0; i < nombres.size(); i++)
		{
				filename = nombres[checked];
				guiEvents.push(new cEventManageConnections);
				ImGui::CloseCurrentPopup();
		}

		eventHappened = true;
	}
	else {
		eventHappened = false;
	}
	return eventHappened;
}


std::vector<std::string> interfaseEventGenerator::extract_keys(std::map<std::string, boost::asio::ip::tcp::socket*> const& input_map) {
	std::vector<std::string> retval;
	for (auto const& element : input_map) {
		retval.push_back(element.first);
	}
	return retval;
}

string interfaseEventGenerator::getFilename() { return filename; }

void interfaseEventGenerator::pushBackEvent()
{
	guiEvents.push(new cEventBack);
}



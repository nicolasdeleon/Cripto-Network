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
	notParsed = true;
	makingChecked = -1;
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
	currentNodes = mySim->getNodes();
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

void interfaseEventGenerator::printInfoWindow(void) {
	//tuve que hacer esto porque sino tardaba mucho tiempo en correr por el for que viene despues, si se quiere probar reemplazar esto en donde diga blockchainlocal.
	static json blockchainlocal = mySim->Nodes[makingChecked]->getBlockChain();
	static POPS caso = POPS::Default;
	static string errorString = "No Hubo Error";
	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();
	if (notParsed){
		blockchainHandler.parseallOk(mySim->Nodes[makingChecked]->getstr(), &errorString);
		notParsed = false;
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(SIZE_SCREEN_X, SIZE_SCREEN_X));

	ImGui::Begin("Informacion", 0, 0);
	static int radio_button=-1;
	for (int i = 0; i < blockchainlocal.size(); i++){
		string blockId = blockchainlocal[i]["blockid"].get<string>();
		ImGui::RadioButton(blockId.c_str(), &radio_button, i);
	}

	if (ImGui::Button("Information"))
	{
		if (radio_button != -1) {
			showBlockInfo(radio_button);
			caso = POPS::Info;
		}
		else {
			//sin esta parte daria un error al querer leer filename con nada guardado.
			printf("Tenes que seleccionar un radio button antes de clickear el boton\n");
			errorString = "Select a radioButton to continue\n";
			caso = POPS::Failed;
		}
		
	}

	if (ImGui::Button("Calcular el Merkle root"))
	{

		if (radio_button != -1) {
			caso = POPS::Merkle;
		}
		else {
			//sin esta parte daria un error al querer leer filename con nada guardado.
			printf("Tenes que seleccionar un radio button antes de clickear el boton\n");
			errorString = "Select a radioButton to continue\n";
			caso = POPS::Failed;
		}
	}

	if (ImGui::Button("Validar el Merkle root"))
	{

		if (radio_button != -1) {
			caso = POPS::ValidateMerkle;
		}
		else {
			//sin esta parte daria un error al querer leer filename con nada guardado.
			printf("Tenes que seleccionar un radio button antes de clickear el boton\n");
			errorString = "Tenes que seleccionar un radio button antes de clickear el boton\n";
			caso = POPS::Failed;
		}

	}

	if (ImGui::Button("Merkle Tree"))
	{

		if (radio_button != -1) {
			vector<vector<string>> tree = blockchainHandler.makeMerkleTree(radio_button);
			printTree(tree);
		}
		else {
			//sin esta parte daria un error al querer leer filename con nada guardado.
			printf("Tenes que seleccionar un radio button antes de clickear el boton\n");
			errorString = "Tenes que seleccionar un radio button antes de clickear el boton\n";
			caso = POPS::Failed;
		}
	}

	if (displayTree.show) {
		//ImGui::SetNextWindowPos(ImVec2(400, 10));  //lo quite por conveniencia podemos ponerlo despues pero en una pos mas comoda
		ImGui::SetNextWindowSize(ImVec2(500, 450));
		ImGui::Begin("Merkel Tree", 0, 0);

		ImGui::BeginChild("Merkle Tree", ImVec2(470, 370), true, ImGuiWindowFlags_None);
		const char* tree = displayTree.tree.c_str();
		ImGui::TextUnformatted(tree, tree + displayTree.tree.size());
		ImGui::EndChild();


		if (ImGui::Button("Close")){
			displayTree.show = false;
		}
		ImGui::End();
	}


	if (ImGui::Button("Volver"))
	{
		guiEvents.push(new cEventBack);
	}

	switch(caso)
	{
	case POPS::Info:
			ImGui::OpenPopup("Block Info");
			if (ImGui::BeginPopupModal("Block Info", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(displayInfo.blockId.c_str());
				ImGui::Text(displayInfo.previousBlockId.c_str());
				ImGui::Text(displayInfo.NTransactions.c_str());
				ImGui::Text(displayInfo.BlockNumber.c_str());
				ImGui::Text(displayInfo.nonce.c_str());
				if (ImGui::Button("Ok", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); caso = POPS::Default; }
				ImGui::EndPopup();
			}
			break;
	case POPS::Merkle:
		ImGui::OpenPopup("Merkle root");

		if (ImGui::BeginPopupModal("Merkle root", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
				string ourMakleRoot = blockchainHandler.makeMerkleTree(radio_button).back().back();
				ImGui::Text(("Nuestra merkle root: " + ourMakleRoot).c_str());
				ImGui::Text(("La merkle root del bloque: " + blockchainHandler.getMerkleroot()).c_str());
				ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				caso = POPS::Default;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		break;
	case POPS::ValidateMerkle:
		ImGui::OpenPopup("Merkle root");

		if (ImGui::BeginPopupModal("Merkle root", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			string ourMakleRoot = blockchainHandler.makeMerkleTree(radio_button).back().back();
			string blockMakleRoot = blockchainHandler.getMerkleroot();

			if (ourMakleRoot == blockMakleRoot) 
			{
				ImGui::Text("La makleroot del bloque es correcta!");
			}
			else 
			{
				ImGui::Text("La makleroot del bloque NO es correcta!");
			}
		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			caso = POPS::Default;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
		}
		break;

	case POPS::Default:
		break;

	case POPS::Failed:
		ImGui::OpenPopup("Failed");

		if (ImGui::BeginPopupModal("Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(errorString.c_str());
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				errorString = "";
				caso = POPS::Default;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		break;
	default:
		break;
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

	for (int i = 0; i < currentNodes.size(); i++)
	{
		ImGui::RadioButton(currentNodes[i]->getAddress().c_str(), &makingChecked, i);
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

	//if (ImGui::Button("Info") && checked != -1) {
	//	guiEvents.push(new cEventCreateNodeScreen);
	////ImGui::OpenPopup("Info");
	//}

	//if (ImGui::BeginPopupModal("Info", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	//{
	//	static int radio_button;
	//	for (int i = 0; i < mySim->Nodes[MY_NODE_FULL]->getBlockChain().size(); i++)
	//	{
	//		string blockId = mySim->Nodes[MY_NODE_FULL]->getBlockChain()[i]["blockid"].get<string>();
	//		ImGui::RadioButton(blockId.c_str(), &radio_button, i);
	//	}

	//	showBlockInfo(radio_button);
	//	ImGui::Text(displayInfo.blockId.c_str());
	//	ImGui::Text(displayInfo.previousBlockId.c_str());
	//	ImGui::Text(displayInfo.NTransactions.c_str());
	//	ImGui::Text(displayInfo.BlockNumber.c_str());
	//	ImGui::Text(displayInfo.nonce.c_str());
	//	/*if (ImGui::Button("Seleccionar otra block chain")) {
	//		EventoActual = Event::BackToMainMenu;
	//		eventHappened = true;
	//	}*/
	//	if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
	//	ImGui::EndPopup();
	//}

	ImGui::SameLine();
	if (ImGui::Button("Info") && makingChecked != -1) {
		guiEvents.push(new cEventInfo);
	}

	if (ImGui::Button("Delete Node") && makingChecked != -1)
		ImGui::OpenPopup("Del Node");

	if (ImGui::BeginPopupModal("Del Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure? a node deletion is permanent!\n");
		if (ImGui::Button("Yes", ImVec2(120, 0))) {
			mySim->deleteNode(currentNodes[makingChecked]->getIP(), currentNodes[makingChecked]->getPort());
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


void interfaseEventGenerator::showBlockInfo(int index) {
	displayInfo.blockId = "Block Id: " + mySim->Nodes[makingChecked]->getBlockChain()[index]["blockid"].get<string>();
	displayInfo.previousBlockId = "Previous block id: " + mySim->Nodes[makingChecked]->getBlockChain()[index]["previousblockid"].get<string>();
	displayInfo.NTransactions = "Number of transactions: " + to_string(mySim->Nodes[makingChecked]->getBlockChain()[index]["nTx"].get<int>());
	displayInfo.BlockNumber = "Block number: " + to_string(index);
	displayInfo.nonce = "Nonce: " + to_string(mySim->Nodes[makingChecked]->getBlockChain()[index]["nonce"].get<int>());
	displayInfo.show = true;
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
		/*
		static int i0 = 123;
        ImGui::InputInt("input int", &i0);
		ImGui::Text("Please select the target node's IP and port:\n\n");
		ImGui::Separator();

		static int checkedCnx = -1;

		for (int i = 0; i < currentNodes.size(); i++)
		{
			if (currentNodes[i]->getAddress() != currentNodes[checked]->getAddress()) {
				ImGui::RadioButton(currentNodes[i]->getAddress().c_str(), &checkedCnx, i);
			}	
		}
		*/

		static char pubId[8];
		ImGui::InputText("Public Id", pubId, size_t(8), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);


		ImGui::Text("Please select the size of the transaction: ");
		static int amount;
		ImGui::InputInt("EDA$", &amount);

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			
			vector<string> target_adresses;
			target_adresses.push_back(pubId);
			vector<int> amounts;
			amounts.push_back(amount);

			mySim->sendTransaction(currentNodes[checked]->getAddress(), target_adresses, amounts);
			ImGui::CloseCurrentPopup();
			//checkedCnx = -1;
		}


		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup(); 
			//checkedCnx = -1;
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

	ImGui::SetNextWindowPos(ImVec2(100, 120));
	ImGui::SetNextWindowSize(ImVec2(300, 300));
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
	ImGui::BeginChild(".json files in current folder", ImVec2(200, 200), true, ImGuiWindowFlags_None);

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

void interfaseEventGenerator::pushCnxEvent() 
{
	guiEvents.push(new cEventManageConnections);;
}


void interfaseEventGenerator::printTree(vector<vector<string>> Tree) {
	int H = Tree[0].size() * 2;
	int W = Tree.size() * 2;
	vector<vector<string>> stringMap(H, vector<string>(W, "     "));
	displayTree.tree = "";
	displayTree.show = true;
	float counter = H;
	for (int i = 0; i < Tree.size(); i++) {
		counter = counter / 2;
		int offset = 0;
		for (int j = 0; j < Tree[Tree.size() - i - 1].size(); j++) {
			if (!((int(counter) * (j + 1) + offset) % (int(counter) * 2)))
				offset = offset + counter;
			stringMap[counter * (j + 1) + offset][i * 2] = "---" + Tree[Tree.size() - i - 1][j];
		}
	}

	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			displayTree.tree += stringMap[i][j];
		}
		displayTree.tree += "\n\r";
	}

}
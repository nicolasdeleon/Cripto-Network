#include "FullNode.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

void createConection(string& origin_address, string& destiny_address, vector<GenericNode*>& Nodes);
void visitar(int nodo_a_visitar, vector<NodeInfo>& pingedNodes);

FullNode::FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/eda_coin/send_block");
	permitedPaths.push_back("/eda_coin/send_tx");
	permitedPaths.push_back("/eda_coin/send_merkle_block");
	permitedPaths.push_back("/eda_coin/send_filter");
	permitedPaths.push_back("eda_coin/get_blocks");
	permitedPaths.push_back("/eda_coin/get_blocks");
	permitedPaths.push_back("/eda_coin/get_block_header");
	permitedPaths.push_back("eda_coin/get_block_header");
	permitedPaths.push_back("/eda_coin/PING");
	permitedPaths.push_back("/eda_coin/NETWORK_LAYOUT");

	type = NodeType::FULL;

	NodeInfo thisInfo;
	thisInfo.ip = ip;
	thisInfo.puerto = port;
	pingedNodes.push_back(thisInfo);
	currState = NodeState::IDLE;

	al_init();
	queue = al_create_event_queue();	
	timer = al_create_timer(0.01);
	al_register_event_source(queue, al_get_timer_event_source(timer));
	int i = (rand() % 990) + 10;
	countdown = 10 * i;
	al_start_timer(timer);

	cout << port << "- countdown = " << countdown << "ms" << endl;
}

// Funcion para enviar pedido desde el nodo Full
void FullNode::send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant) {

	switch (id) {
	case(MessageIds::BLOCK):
		sendBlock("send_block", ip, port, to_string(block_id));
		break;
	case(MessageIds::TRANSACTION):
		sendTX("send_tx", ip, port, { 15 }, { "32423" });
		break;
	case(MessageIds::MERKLE_BLOCK):
		sendMklBlock("send_merkle_block", ip, port, to_string(block_id), 2);
		break;
	case(MessageIds::GET_BLOCKS):
		getBlocks("get_blocks", ip, port, std::to_string(block_id), cant);
		break;
	default:
		std::cout << "Error format in send package: Wrong id type" << std::endl;
		break;
	}
}

// Funcion que responde a un pedido desde el nodo Full
void FullNode::dispatch_response(string path, string incoming_address, unsigned int block_id, unsigned int count) {
	std::cout << "response_dispatch()" << std::endl;

	json response;

	response["status"] = "true";

	if (path == "/eda_coin/send_block") {
		response["result"] = "null";
	}
	else if (path == "/eda_coin/send_tx") {
		cout << incoming_address << " $" << count << endl;
		response["result"] = "null";
	}
	else if (path == "/eda_coin/send_merkle_block") {
		response["result"] = "null";
	}
	else if (path == "/eda_coin/send_filter") {
		response["result"] = "null";
	}
	else if (path == "/eda_coin/get_blocks" || path == "eda_coin/get_blocks") {
		int blocks_left = 0;
		bool allOk = false;
		for (auto block : blockChain) {
			if (block["blockid"] == block_id) {
				allOk = true;
				blocks_left = count--;
				response["result"].push_back(block);
			}
			else if (blocks_left) {
				blocks_left--;
				response["result"].push_back(block);
			}
		}

		if (allOk = false || blocks_left) {
			response.clear();
			response["status"] = false;
			response["result"] = 2;
		}
	}
	else if (path == "/eda_coin/get_block_header" || path == "eda_coin/get_block_header") {
		bool allOk = false;
		if (blockChain.size()) {
			for (auto block : blockChain) {
				if (block["blockid"] == block_id) {
					allOk = true;
					response["result"]["blockid"] = block["blockid"];
					response["result"]["height"] = block["height"];
					response["result"]["merkleroot"] = block["merkleroot"];
					response["result"]["nTx"] = block["nTx"];
					response["result"]["nonce"] = block["nonce"];
					response["result"]["previousblockid"] = block["previousblockid"];
				}
			}
			if (!allOk) {
				response.clear();
				response["status"] = false;
				response["result"] = 2;
			}
		}
		else {
			response.clear();
			response["status"]["blockid"] = "00000000";
		}
	}
	else if (path == "/eda_coin/PING") {
		cout << incoming_address << " pinged " << port << endl;
		switch (currState) {
		case NodeState::IDLE:
			response["status"] = "NETWORK_NOTREADY";
			currState = NodeState::WAITING_NETW_LAYOUT;
			break;
		case NodeState::COL_NETW_MEMBS:
			response["status"] = "NETWORK_READY";
			currState = NodeState::NETW_CREATED;
			algoritmoParticular();
			break;
		case NodeState::WAITING_NETW_LAYOUT:
			response["status"] = "NETWORK_READY";
			break;
		case NodeState::NETW_CREATED:
			response["status"] = "NETWORK_READY";
			break;
		case NodeState::WAITING_LAYOUT_RESPONSE:
			//response["status"] = "NETWORK_READY";
			break;
		case NodeState::WAITING_PING_RESPONSE:
			//response["status"] = "NETWORK_READY";
			break;
		}
	}
	else if (path == "/eda_coin/NETWORK_LAYOUT") {
		if (currState == NodeState::WAITING_NETW_LAYOUT) {
			response["status"] = "null";
			currState = NodeState::NETW_CREATED;
			executeLayout();
		}
	}
	else {
		std::cout << "NUNCA DEBERIA LLEGAR ACA" << std::endl;
	}

	std::cout << "Respuesta del servidor " << createAddress(ip, port) << " al pedido:" << endl << response.dump() << std::endl;
	answers[incoming_address] = wrap_package(response.dump());

}

FullNode::~FullNode() {

}

void FullNode::sendMklBlock(string path, string outIp, int outPort, string blockId, int tx_pos) {

	json to_send_;
	to_send_["blockid"] = blockId /*blockChain[blockId]*/;

	for (auto block : blockChain) {
		// TODO: DECIRTE QUE ESTAS FLASHEANDO SI NO ENCUENTRA EL BLOCK ID
		if (block["blockid"] == blockId) {
			to_send_["tx"][tx_pos] = block["tx"];
			to_send_["txPos"] = tx_pos;
			to_send_["merklePath"];
			for (auto id : makeMerklePath(blockChain, block["tx"][tx_pos]["txid"])) {
				to_send["merklePath"].push_back({ "id", id });
			}
		}
		else {
			cout << "No existe el blockid seleccionado";
		}
	}

	cout << to_send_.dump() << endl;
	client.methodPost(path, outIp, outPort, to_send_);
}

void FullNode::sendTX(string path, string outIp, int outPort, vector<int> amounts, vector<string> publicIds) {

	to_send.clear();
	int nTxout = amounts.size();

	if (amounts.size() == publicIds.size()) {
		to_send["nTxin"] = 1;
		to_send["nTxout"] = nTxout;
		to_send["txid"] = "7B857A14";
		to_send["vout"] = {};
		for (int i = 0; i < nTxout; i++) {}
		to_send["vin"] = {};
		to_send["vin"].push_back({ { "blockid", "00000BDE" },
			{ "outputIndex", 4 },
			{ "signature", "000009B7" },
			{ "txid", "00000EBA" } });
	}
	else{
		//pifiaste mache
		to_send["loco"] = "pifiaste";
	}
	
	client.methodPost(path, outIp, outPort, to_send);
}



void FullNode::sendFilter(string path, string outIp, int outPort) {
	json temp;
	temp["Key"] = "Node 1";
	client.methodPost(path, outIp, outPort, temp);
}

void FullNode::sendBlock(string path_, string outIp, int outPort, string blockId) {

	json temp;

	for (auto block : blockChain) {
		if (block["blockid"] == blockId) {
			temp = block;
		}
		else {
			//pifiaste mache
			to_send.clear();
			cout << "pifiaste, no existe el blockid seleccionado";
		}
	}
	client.methodPost(path_, outIp, outPort, temp);
}

void FullNode::getBlocks(string path_, string outIp, int outPort, string blockId, int numBlocks) {
	client.methodGet(path_, outIp, outPort, blockId, numBlocks);
}


string FullNode::hexCodexASCII(unsigned int number) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << number;

	return ss.str();
}

unsigned int FullNode::generateID(unsigned char* str){
	unsigned int id = 0;
	int c;
	while (c = *str++)
		id = c + (id << 6) + (id << 16) - id;
	return id;
}


vector<string> FullNode::makeMerklePath(int blockNumber, string txid) {


	vector<vector<string>> levels;
	
	// pre-Hojas 
	vector<string> currentLevel;

	vector<string> prehashIDs;
	for (auto transaction : blockChain[blockNumber]["tx"]) {
		string id;
		for (auto tx : transaction["vin"]) {
			id += string(tx["txid"].get<string>());
		}
		prehashIDs.push_back(id);
	}
	if (prehashIDs.size() % 2) {
		prehashIDs.push_back(prehashIDs.back());
	}


	// Hojas

	currentLevel.clear();
	for (string oldId : prehashIDs) {

		unsigned char* oldID_char = new unsigned char[oldId.length() + 1]; //Copiado de stack overflow 
		strcpy((char*)oldID_char, oldId.c_str());

		unsigned int newID = generateID(oldID_char);
		currentLevel.push_back(hexCodexASCII(newID));
	}
	// Borrar lo de abajo
	if (currentLevel.size() % 2) {
		currentLevel.push_back(levels[1].back());
	}
	levels.push_back(currentLevel);


	// Ramas 



	while (levels.back().size() > 1) {

		currentLevel.clear();

		if (!(levels.back().size() % 2)) {
			// Uno los hashes anteriores en el actual
			for (int i = 1; i < levels.back().size(); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
		}
		else {
			int i;
			for (i = 1; i < (levels.back().size() - 1); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
			currentLevel.push_back(levels.back()[i - 1] + levels.back()[i - 1]);
		}

		//re-hasheo el nivel actual

		for (int i = 0; i < currentLevel.size(); i++) {

			unsigned char* oldID = new unsigned char[currentLevel[i].length() + 1]; //Copiado de stack overflow 
			strcpy((char*)oldID, currentLevel[i].c_str());

			unsigned int newID = generateID(oldID);
			currentLevel[i] = hexCodexASCII(newID);
		}

		//Agrego el nivel actual al vector
		levels.push_back(currentLevel);
	}


	vector<string> merklePath;
	int index = -1;

	for (int i = 0; i < levels[0].size(); i++) {
		if (levels[0][i] == txid) {
			index = i;
		}
	}

	for (int j = 0; levels[j].size() > 1; j++, index / 2) {
		if (index % 2) {
			if (index != levels[j].size()) {
				merklePath.push_back(levels[j][index + 1]);
			}
			else {
				merklePath.push_back(levels[j][index]);
			}
		}
		else {
			merklePath.push_back(levels[j][index - 1]);
		}
	}

	return merklePath;
}



void FullNode::algoritmoParticular(void)
{
	json layoutJson;
	for (NodeInfo node : pingedNodes) {
		layoutJson["nodes"].push_back(createAddress(node.ip, node.puerto));
	}

	cout << layoutJson << endl;
	
	if (pingedNodes.size() > 1) {

		if (pingedNodes.size() > 2) {

			for (int num_nodo = 0; num_nodo < pingedNodes.size(); num_nodo++)
			{
				//imprimo un aviso nomas
				if (pingedNodes[num_nodo].connections < 2)
				{

					//hago las conexiones hasta que el numero de conexiones sea 2 
					while (pingedNodes[num_nodo].connections < 2)
					{
						int node_to_connect;
						bool already_conected;
						//le doy un numero arbitrario que no sea justo su mismo numero para evitar que se conecte consigo mismo, ni tampoco sea una conexion que ya se establecio.
						do {
							already_conected = false;
							node_to_connect = rand() % pingedNodes.size() + 0;
							for (int i = 0; i < pingedNodes[num_nodo].conectedWith.size(); i++)
							{
								if (node_to_connect == pingedNodes[num_nodo].conectedWith[i])
								{
									already_conected = true;
								}
							}
						} while (num_nodo == node_to_connect || already_conected == true);

						//creo las direcciones de salida y llegada de la conexion
						layoutJson["edges"].push_back({ {"target1", createAddress(pingedNodes[num_nodo].ip, pingedNodes[num_nodo].puerto)},
													  {"target2", createAddress(pingedNodes[node_to_connect].ip, pingedNodes[node_to_connect].puerto)} });
						pingedNodes[num_nodo].conectedWith.push_back(node_to_connect);
						pingedNodes[node_to_connect].conectedWith.push_back(num_nodo);
						pingedNodes[num_nodo].connections++;
						pingedNodes[node_to_connect].connections++;
					}
				}
			}

		}
		else {
			layoutJson["edges"].push_back({ {"target1", createAddress(pingedNodes[0].ip, pingedNodes[0].puerto)},
										  { "target2", createAddress(pingedNodes[1].ip, pingedNodes[1].puerto)} });
		}

		cout << "json de la red:" << endl << endl << layoutJson.dump(2) << endl;

		//compruebo que sea conexo el grafo
		//es_conexo()

		layout = layoutJson;



	}
	
}


bool FullNode::es_conexo(void)
{
	
	//algoritmo para revisar si es conexo.
	//empiezo en nodo 0
	visitar(0, pingedNodes);
	for (int i = 0; i < pingedNodes.size(); i++)
	{
		if (pingedNodes[i].visitado == false)
		{
			return false;
		}
	}
	return true;
}

void createConection(string& origin_address, string& destiny_address, vector<GenericNode*>& Nodes)
{
	for (GenericNode* node : Nodes) {
		if (node->getAddress() == origin_address) {
			node->addConnection(destiny_address);
		}
	}
}

void visitar(int nodo_a_visitar, vector<NodeInfo>& pingedNodes)
{
	if (pingedNodes[nodo_a_visitar].visitado == false)
	{
		pingedNodes[nodo_a_visitar].visitado = true;
		for (int i = 0; i < pingedNodes[nodo_a_visitar].conectedWith.size(); i++)
		{
			visitar(pingedNodes[nodo_a_visitar].conectedWith[i], pingedNodes);
		}
	}
}

void FullNode::doPolls() {
	getNodeIoContext().poll();
	curlPoll();
	static bool b00l = true;
	switch (currState) {
	case NodeState::NETW_CREATED:
		break;
	case NodeState::IDLE:
		ALLEGRO_EVENT ev;

		if (al_get_next_event(queue, &ev) && ev.type == ALLEGRO_EVENT_TIMER) {

			countdown -= 10;

			//cout << countdown << endl;
			if (!countdown) {
				cout << port << " done!" << endl;
				if (timer) {
					al_destroy_timer(timer);
				}
				if (queue) {
					al_destroy_event_queue(queue);
				}
				currState = NodeState::COL_NETW_MEMBS;
			}
		}
		break;
	case NodeState::COL_NETW_MEMBS:
		if (pingedNodes.size() < genesisNodes.size()) {
			pingNodes();
			currState = NodeState::WAITING_PING_RESPONSE;
		}
		else {
			algoritmoParticular();
			currState = NodeState::SENDING_LAYOUTS;
		}
		break;
	case NodeState::WAITING_PING_RESPONSE:
		if (!client.getAnswer().empty()) {

			string dummy = client.getAnswer()["status"];

			if (client.getAnswer()["status"] == "NETWORK_READY") {
				cout << createAddress(ip, port) << " received NW READY responde from " << pingingNodeAdress << endl;
				addConnection(pingingNodeAdress);
				algoritmoParticular();
				currState = NodeState::SENDING_LAYOUTS;
				client.clearAnswer();
			}
			else if (client.getAnswer()["status"] == "NETWORK_NOTREADY") {
				NodeInfo newNode;
				newNode.puerto = stoi(pingingNodeAdress.substr(pingingNodeAdress.find(":") + 1));
				newNode.ip = pingingNodeAdress.substr(0, pingingNodeAdress.find(":"));
				pingedNodes.push_back(newNode);
				currState = NodeState::COL_NETW_MEMBS;
				client.clearAnswer();
			}
		}
		break;
	case NodeState::WAITING_LAYOUT_RESPONSE:
		if (!client.getAnswer().empty()) {
			pingedNodes.pop_back();
			client.clearAnswer();
			currState = NodeState::SENDING_LAYOUTS;
		}
		break;
	case NodeState::SENDING_LAYOUTS:
		if (pingedNodes.size() > 1) {
			client.methodPost("NETWORK_LAYOUT", pingedNodes.back().ip, pingedNodes.back().puerto, layout);
			currState = NodeState::WAITING_LAYOUT_RESPONSE;
		}
		else {
			incoming_json = layout;
			executeLayout();
			currState = NodeState::NETW_CREATED;
		}
		break;
	case NodeState::WAITING_NETW_LAYOUT:
		break;
	case NodeState::APPENDING:
		if (!client.getAnswer().empty()) {
			string dummy = client.getAnswer()["status"];
			std::string ping_response_address = neighbour_iterator->first;
			if (client.getAnswer()["status"] == "NETWORK_READY") {
				cout << createAddress(ip, port) << " received NW READY response from " << ping_response_address << endl;
			}
			else {
				cout << createAddress(ip, port) << " received OTHER response from " << ping_response_address << endl;
				remove_address.push_back(ping_response_address);
			}
			neighbour_iterator++;
			client.clearAnswer();
			if (neighbour_iterator == connections.end()) {
				for (string address : remove_address) {
					deleteConnection(address);
				}
				currState = NodeState::NETW_CREATED;
				endAppend();
			}
			else {
				// PING
				json emptyJson;
				emptyJson.clear();
				std::string ping_address = neighbour_iterator->first;
				string ping_ip = get_address_ip(ping_address);
				unsigned int ping_port = get_address_port(ping_address);
				cout << createAddress(ip, port) << " doing ping on " << ping_address << endl;
				client.methodPost("PING", ping_ip.c_str(), ping_port, emptyJson);
			}
		}
		break;
	}
}

void FullNode::pingNodes() {

	int randNum, targetPort;
	bool alreadyPinged;
	int count = 0;

	do {

		alreadyPinged = false;
		randNum = rand() % genesisNodes.size();
		targetPort = stoi(genesisNodes[randNum]);

		for (NodeInfo node : pingedNodes) {

			if (targetPort == node.puerto) {
				alreadyPinged = true;
			}

		}

	} while (alreadyPinged);


	
	json emptyJson;
	emptyJson.clear();

	pingingNodeAdress = createAddress("127.0.0.1", targetPort);

	cout << createAddress(ip, port) << " tried to ping " << targetPort << endl;
	client.methodPost("PING", "127.0.0.1", targetPort, emptyJson);
}

void FullNode::executeLayout()
{
	string thisNodesAddress = createAddress(ip, port);

	for (auto connection : incoming_json["edges"]) {
		if (connection["target1"] == thisNodesAddress) {
			addConnection(connection["target2"]);
		}
		else if (connection["target2"] == thisNodesAddress) {
			addConnection(connection["target1"]);
		}
	}
}

void FullNode::startAppend() {
	neighbour_iterator = connections.begin();
	currState = NodeState::APPENDING;
	// PING
	json emptyJson;
	emptyJson.clear();
	std::string ping_address = neighbour_iterator->first;
	string ping_ip = get_address_ip(ping_address);
	unsigned int ping_port = get_address_port(ping_address);
	cout << createAddress(ip, port) << " doing ping on " << ping_address << endl;
	client.methodPost("PING", ping_ip.c_str(), ping_port, emptyJson);
}

void FullNode::endAppend() {
	neighbour_iterator = connections.begin();
	if (connections.size() == 0) {
		cout << createAddress(ip, port) << "has no connections!" << endl;
		return;
	}
	unsigned int node_to_connect = rand() % connections.size() + 0;
	for (unsigned int i = 0; i < node_to_connect; i++) {
			neighbour_iterator++;
	}

	string node_to_connect_addres = neighbour_iterator->first;

	cout << createAddress(ip, port) << " is asking " << node_to_connect_addres << " for blocks..." << endl;
	// TODO: UN HARDCODE THIS
	string blocks_ip = get_address_ip(node_to_connect_addres);
	unsigned int blocks_port = get_address_port(node_to_connect_addres);
	getBlocks("get_blocks", blocks_ip, blocks_port, "00000000", 33);
	neighbour_iterator = connections.begin();
}

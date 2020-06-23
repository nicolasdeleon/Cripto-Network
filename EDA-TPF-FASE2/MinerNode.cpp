#include "MinerNode.h"

MinerNode::MinerNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : FullNode(io_context, ip, port) {
	type = NodeType::MINER;
}

MinerNode::~MinerNode() {

}

void MinerNode::mineBlock (){

	json blockToSend;
	json minerTsx = createMiningFee();
	nextBlockTSXs.push_back(minerTsx);
	for (json tsx : nextBlockTSXs) {

		blockToSend["tx"].push_back(tsx);

	}
	vector<vector<string>> merkleTree = blockchainHandler.makeMerkleTree(blockToSend);
	
	unsigned int height = merkleTree.size();
	string merkleRoot = merkleTree.back()[0];
	unsigned int nTx = blockToSend["tx"].size();
	string nonce = blockchainHandler.hexCodexASCII(rand() % 345);

	blockToSend["height"] = height;
	blockToSend["merkleroot"] = merkleRoot;
	blockToSend["nTx"] = nTx;
	blockToSend["nonce"] = nonce;

	string tmp = hexCodexASCII(height) + merkleRoot + hexCodexASCII(nTx) + nonce;

	unsigned char* temp = new unsigned char[tmp.length() + 1]; //Copiado de stack overflow 
	strcpy((char*)temp, tmp.c_str());

	string blockID = hexCodexASCII(generateID(temp));

	blockToSend["blockid"] = blockID;

	if (blockchainHandler.chainSize()) {
		blockToSend["previousblockid"] = blockchainHandler.BlockChainJSON[0];
	}
	else {
		blockToSend["previousblockid"] = "00000000";
	}

	blockchainHandler.BlockChainJSON.push_back(blockToSend);
	mensajesRecibidos.push_back(incoming_json.dump());
	nextBlockTSXs.clear();
	
	vector<string> neighbors = extract_keys(connections);
	BLKfloodRequest newRequest(blockToSend, neighbors);
	cout << blockToSend.dump(2) << endl;

	pendingBlockFloodRequests.push(newRequest);

}

json MinerNode::createMiningFee() {

	json retVal;

	retVal["nTxin"] = 1;
	retVal["nTxout"] = 1;
	retVal["txid"] = "7B857A14";
	retVal["vout"] = {};
	retVal["vin"] = {};
	retVal["vout"].push_back(
		{
			{"amount", 10},
			{"publicid", "AAAAAAAA"}
		}
	);
	
	return retVal;
}

void MinerNode::processTsx(string& incoming_address, json& incoming_json) {

	bool alreadyGotIt = false;

	for (int i = 0; i < mensajesRecibidos.size() && !alreadyGotIt; i++)
	{
		if (incoming_json.dump() == mensajesRecibidos[i])
			alreadyGotIt = true;
	}
	if (!alreadyGotIt)
	{
		std::cout << "ok soy " << ip << ":" << port << " guardare tu mensaje que viene de:" << incoming_address << endl;
		mensajesRecibidos.push_back(incoming_json.dump());
		nextBlockTSXs.push_back(incoming_json); //<- A diferencia de full node lo guardamos también en un buffer 
		parseIncoming(incoming_address, incoming_json);
	}
	else
	{
		std::cout << "ok soy " << ip << ":" << port << " ya lo tenia (me lo mando: " << incoming_address << ")" << endl;
	}
}

void MinerNode::doPolls() {
	try {

		getNodeIoContext().poll();
		curlPoll();
		static bool b00l = true;
		ALLEGRO_EVENT ev;
		switch (currState) {
		case NodeState::NETW_CREATED:


			if (nextBlockTSXs.size() && al_get_next_event(AlEvQueue, &ev) && ev.type == ALLEGRO_EVENT_TIMER) {
				mineBlock();
				currState = NodeState::FLOOD_BLOCK;
			}

			if (pendingFloodRequests.size()) {
				currState = NodeState::FLOOD;
			}
			else if (pendingBlockFloodRequests.size()) {
				currState = NodeState::FLOOD_BLOCK;
			}

			break;
		case NodeState::FLOOD:
			flood_transaction();
			break;
		case NodeState::WAITING_FLOOD_RESPONSE:
			if (!client.getAnswer().empty()) {
				//string s = client.getAnswer().dump(1);
				client.clearAnswer();
				currState = NodeState::FLOOD;
			}
			break;
		case NodeState::FLOOD_BLOCK:
			flood_block();
			break;
		case NodeState::WAITING_BLKFLOOD_RESPONSE:
			if (!client.getAnswer().empty()) {
				//string s = client.getAnswer().dump(1);
				client.clearAnswer();
				currState = NodeState::FLOOD_BLOCK;
			}
			break;
		case NodeState::IDLE:

			if (al_get_next_event(AlEvQueue, &ev) && ev.type == ALLEGRO_EVENT_TIMER) {

				countdown -= 10;

				//cout << countdown << endl;
				if (!countdown) {
					//cout << port << " done!" << endl;
					timer = al_create_timer(15);
					al_start_timer(timer);
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
					//cout << createAddress(ip, port) << " received NW READY responde from " << pingingNodeAdress << endl;
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
					//cout << createAddress(ip, port) << " received NW READY response from " << ping_response_address << endl;
				}
				else {
					//cout << createAddress(ip, port) << " received OTHER response from " << ping_response_address << endl;
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
					//cout << createAddress(ip, port) << " doing ping on " << ping_address << endl;
					client.methodPost("PING", ping_ip.c_str(), ping_port, emptyJson);
				}
			}
			break;
		}

	}
	catch (boost::system::system_error e) {
		std::cout << e.code() << std::endl;
	}
}
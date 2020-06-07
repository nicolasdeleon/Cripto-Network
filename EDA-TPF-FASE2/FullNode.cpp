#include "FullNode.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

void createConection(string& origin_address, string& destiny_address, vector<GenericNode*>& Nodes);
void visitar(int nodo_a_visitar, vector<NodeInfo>& PingedNodes);

FullNode::FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/eda_coin/send_block");
	permitedPaths.push_back("/eda_coin/send_tx");
	permitedPaths.push_back("/eda_coin/send_merkle_block");
	permitedPaths.push_back("/eda_coin/send_filter");
	permitedPaths.push_back("/eda_coin/get_blocks");
	permitedPaths.push_back("/eda_coin/get_block_header");
	type = NodeType::FULL;
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
	else if (path == "/eda_coin/get_blocks") {
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
	else if (path == "/eda_coin/get_block_header") {
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
	else {
		std::cout << "NUNCA DEBERIA LLEGAR ACA" << std::endl;
	}

	std::cout << "Respuesta del servidor al pedido:" << endl << response.dump() << std::endl;
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



void FullNode::algoritmoParticular(vector<GenericNode*>& Nodes)
{
	for (int num_nodo = 0 ; num_nodo < PingedNodes.size(); num_nodo++ )
	{
		//imprimo un aviso nomas
		if (PingedNodes[num_nodo].connections >= 2)
		{
			cout << "A este nodo no lo conecto con nadie porque ya tiene 2 conexiones" << endl;
		}
		else
		{
			//imprimo un aviso nomas
			if (PingedNodes[num_nodo].connections == 1)
			{
				cout << "A este nodo lo conectare una unica vez ya que tiene una conexion" << endl;
			}
			//hago las conexiones hasta que el numero de conexiones sea 2 
			while (PingedNodes[num_nodo].connections < 2)
			{
				int node_to_connect;
				bool already_conected;
				//le doy un numero arbitrario que no sea justo su mismo numero para evitar que se conecte consigo mismo, ni tampoco sea una conexion que ya se establecio.
				do{
					already_conected = false;
					node_to_connect = rand() % PingedNodes.size() + 0;
					for (int i = 0; i < PingedNodes[num_nodo].conectedWith.size(); i++)
					{
						if (node_to_connect == PingedNodes[num_nodo].conectedWith[i])
						{
							already_conected = true;
						}
					}
				} while (num_nodo == node_to_connect || already_conected == true);

				//creo las direcciones de salida y llegada de la conexion
				string origin_address = createAddress(PingedNodes[num_nodo].ip, PingedNodes[num_nodo].puerto);
				string destiny_address = createAddress(PingedNodes[node_to_connect].ip, PingedNodes[node_to_connect].puerto);
				createConection(origin_address, destiny_address, Nodes);
				PingedNodes[num_nodo].conectedWith.push_back(node_to_connect);
				//invierto el orden de las direcciones para que la comunicacion se establezca en el sentido inverso tambien.
				createConection(destiny_address, origin_address, Nodes);
				PingedNodes[node_to_connect].conectedWith.push_back(num_nodo);
				//imprimo un aviso nomas
				cout << "A este nodo (el nodo num:" <<num_nodo << ") lo conecto con: " << node_to_connect << endl;
				//aumento el contador de conexiones de ambos nodos.
				PingedNodes[num_nodo].connections++;
				PingedNodes[node_to_connect].connections++;
			}
		}
	}
	//compruebo que sea conexo el grafo
	if (es_conexo())
	{
		cout << "Es conexo" << endl;
	}
	else
	{
		cout << "No es conexo" << endl;
	}
	
}


bool FullNode::es_conexo(void)
{
	
	/*for (int i = 0; i < PingedNodes.size(); i++)
	{
		for (int j = 0; j < PingedNodes[i].conectedWith.size(); j++)
		{
			cout << PingedNodes[i].conectedWith[j] << endl;
		}
	}*/

	//algoritmo para revisar si es conexo.
	//empiezo en nodo 0
	visitar(0, PingedNodes);
	for (int i = 0; i < PingedNodes.size(); i++)
	{
		if (PingedNodes[i].visitado == false)
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

void visitar(int nodo_a_visitar, vector<NodeInfo>& PingedNodes)
{
	if (PingedNodes[nodo_a_visitar].visitado == false)
	{
		PingedNodes[nodo_a_visitar].visitado = true;
		for (int i = 0; i < PingedNodes[nodo_a_visitar].conectedWith.size(); i++)
		{
			visitar(PingedNodes[nodo_a_visitar].conectedWith[i], PingedNodes);
		}
	}
}
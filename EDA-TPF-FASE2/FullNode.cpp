#include "FullNode.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdio.h>



FullNode::FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("eda_coin/eda_coin/send_block");
	permitedPaths.push_back("eda_coin/send_tx");
	permitedPaths.push_back("eda_coin/send_merkle_block");
	permitedPaths.push_back("eda_coin/send_filter");
	permitedPaths.push_back("eda_coin/get_blocks");
	permitedPaths.push_back("eda_coin/get_block_header");
	ifstream originalBlockChainjson("blockChain32.json", ifstream::binary);
	if (originalBlockChainjson) {
		originalBlockChainjson >> blockChain;
	}
	to_send.clear();
}

void FullNode::make_response_package(MessageIds id, std::string incoming_address) {
	std::cout << "full node repsonse" << std::endl;
}


void FullNode::send_request(MessageIds id, std::string ip, unsigned int port) {
}


FullNode::~FullNode() {
}

void FullNode::sendMklBlock(string path, string outIp, int outPort, string blockId, int tx_pos) {
	to_send.clear();
	to_send["blockid"] = blockId /*blockChain[blockId]*/;

	for (auto block : blockChain) {
		if (block["blockid"] == blockId) {
			to_send["tx"][tx_pos] = block["tx"];
			to_send["txPos"] = tx_pos;
			to_send["merklePath"];
			for (auto id : makeMerklePath(blockChain, block["tx"][tx_pos]["txid"])) {
				to_send["merklePath"].push_back({ "id", id });
			}
		}
		else {
			//pifiaste mache
			to_send.clear();
			cout << "pifiaste, no existe el blockid seleccionado";
		}
	}

	client.methodPost(path, outIp, outPort, to_send);
}

void FullNode::sendTX(string path, string outIp, int outPort, string blockId, vector<int> amounts, vector<string> publicIds) {

	json TX;
	int nTxout = amounts.size();

	if (amounts.size() == publicIds.size()) {
		TX["nTxin"] = 1;
		TX["nTxout"] = nTxout;
		TX["txid"] = "7B857A14";
		TX["vout"] = {};
		for (int i = 0; i < nTxout; i++)
		TX["vout"].push_back({{"amount", amounts[i]}, {"publicid", publicIds[i]}});
		TX["vin"] = {};
		TX["vin"].push_back({ { "blockid", "00000BDE" },
			{ "outputIndex", 4 },
			{ "signature", "000009B7" },
			{ "txid", "00000EBA" } });
	}
	else{
		//pifiaste mache
		cout << "pifiaste, no me pasaste igual cant de amounts que publicIds";
	}
	
	client.methodPost(path, outIp, outPort, TX);
}

void FullNode::sendFilter(string path, string outIp, int outPort) {
	json temp;
	temp["Key"] = "Node 1";
	client.methodPost(path, outIp, outPort, temp);
}

void FullNode::sendBlock(string outIp, int outPort, string blockId) {

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
	client.methodPost("get_block_header", outIp, outPort, temp);
}

void FullNode::getBlocks(string outIp, int outPort, string blockId, int numBlocks) {
	client.methodGet("get_block_header", outIp, outPort, blockId, numBlocks);
}


string hexCodexASCII(unsigned int number) {
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
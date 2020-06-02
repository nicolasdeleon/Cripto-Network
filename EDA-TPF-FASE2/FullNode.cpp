#include "FullNode.h"
#include <fstream>


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
			//to_send["merklePath"] = 
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
		TX["vin"].push_back({ "blockid", "00000BDE" },
			{ "outputIndex", 4 },
			{ "signature", "000009B7" },
			{ "txid", "00000EBA" });
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
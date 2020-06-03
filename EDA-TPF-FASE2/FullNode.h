#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class FullNode : public GenericNode
{
public:
	FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	virtual void send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant);
	~FullNode();
private:
	void sendMklBlock(string path, string outIp, int outPort, string blockId, int tx_pos);
	void sendTX(string path, string outIp, int outPort, string blockId, vector<int> amounts, vector<string> publicIds);
	void sendFilter(string path, string outIp, int outPort);
	void sendBlock(string path_, string outIp, int outPort, string blockId);
	void getBlocks(string path_, string outIp, int outPort, string blockId, int numBlocks);
	string hexCodexASCII(unsigned int number);
	unsigned int generateID(unsigned char* str);
	vector<string> makeMerklePath(int blockNumber, string txid);
	json blockChain;
	json to_send;
};

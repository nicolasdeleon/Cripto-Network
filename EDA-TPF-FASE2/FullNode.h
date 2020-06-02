#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class FullNode : public GenericNode
{
public:
	FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	virtual void make_response_package(MessageIds id, std::string incoming_address);
	virtual void send_request(MessageIds id, std::string ip, unsigned int port);

	//void sendFilterMssg(string path, string outIp, int outPort);
	void sendMklBlock(string path, string outIp, int outPort, string blockId, int tx_pos);
	void sendTX(string path, string outIp, int outPort, string blockId, vector<int> amounts, vector<string> publicid);
	void sendFilter(string path, string outIp, int outPort);
	void sendBlock(string outIp, int outPort, string blockId);
	void getBlocks(string outIp, int outPort, string blockId, int numBlocks);
	~FullNode();
private:

	json blockChain;
	json to_send;
};

#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class FullNode : public GenericNode
{
public:
	FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	virtual void send_request(MessageIds id, std::string ip, unsigned int port, unsigned int block_id, unsigned int cant, json Json = {});
	~FullNode();
private:
	unsigned int generateID(unsigned char* str);
	vector<string> makeMerklePath(int blockNumber, string txid);
	json blockChain;
	json to_send;
};

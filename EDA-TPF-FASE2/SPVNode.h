#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class SPVNode : public GenericNode
{
public:
	SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	void send_request(MessageIds id, string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant);	
	void doPolls();
	~SPVNode();
private:
	void dispatch_response(string path, string incoming_address, unsigned int block_id, unsigned int count);
};

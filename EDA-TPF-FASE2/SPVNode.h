#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class SPVNode : public GenericNode
{
public:
	SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	virtual void send_request(MessageIds id, string ip, unsigned int port, unsigned int block_id, unsigned int cant, json Json);
	~SPVNode();
private:

};


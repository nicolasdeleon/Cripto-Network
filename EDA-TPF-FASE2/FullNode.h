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
	~FullNode();
private:

};

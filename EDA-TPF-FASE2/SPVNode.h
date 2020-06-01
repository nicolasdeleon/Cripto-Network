#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

class SPVNode : public GenericNode
{
public:
	SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	virtual void make_response_package(MessageIds id, std::string incoming_address);
	virtual void make_send_package(MessageIds id);
	~SPVNode();
private:

};


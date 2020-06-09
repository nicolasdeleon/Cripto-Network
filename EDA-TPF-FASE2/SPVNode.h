#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>


class SPVNode : public GenericNode
{
	enum class NodeState {
		PASS,
		APPENDING,
		FILTERING
	};

public:
	SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	NodeState currState;
	void send_request(MessageIds id, string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant);	
	void doPolls();
	void startAppend();
	~SPVNode();
private:
	void dispatch_response(string path, string incoming_address, unsigned int block_id, unsigned int count);
	void endAppend();
	void startFiltering();
};

#include "FullNode.h"


FullNode::FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/eda_coin/send_block");
	permitedPaths.push_back("/eda_coin/send_tx");
	permitedPaths.push_back("/eda_coin/send_merkle_block");
	permitedPaths.push_back("/eda_coin/send_filter");
	permitedPaths.push_back("/eda_coin/get_blocks");
	permitedPaths.push_back("/eda_coin/get_block_header");
}


void FullNode::send_request(MessageIds id, std::string ip, unsigned int port, unsigned int block_id, unsigned int cant, json Json) {
	switch (id) {
	case(MessageIds::BLOCK):
		client.methodPost("send_block", ip, port, Json);
		break;
	case(MessageIds::TRANSACTION):
		client.methodPost("send_tx", ip, port, Json);
		break;
	case(MessageIds::MERKLE_BLOCK):
		client.methodPost("send_merkle_block", ip, port, Json);
		break;
	case(MessageIds::GET_BLOCKS):
		client.methodGet("get_blocks", ip, port, block_id, cant);
		break;
	default:
		std::cout << "Error format in send package: Wrong id type" << std::endl;
		break;
	}
}


FullNode::~FullNode() {
}
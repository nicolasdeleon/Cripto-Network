#include "SPVNode.h"

SPVNode::SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/send_merkle_block");
}


void SPVNode::send_request(
	MessageIds id, 
	string ip, 
	unsigned int port, unsigned int block_id, unsigned int cant, json Json) {

	switch (id) {
	case(MessageIds::TRANSACTION):
		client.methodPost("send_tx", ip, port, Json);
		break;
	case(MessageIds::FILTER):
		client.methodPost("send_filter", ip, port, Json);
		break;
	case(MessageIds::GET_BLOCK_HEADER):
		client.methodGet("get_block_header", ip, port, block_id, cant);
		break;
	default:
		std::cout << "Error format in send package: Wrong id type" << std::endl;
		break;
	}
}


SPVNode::~SPVNode() {

}

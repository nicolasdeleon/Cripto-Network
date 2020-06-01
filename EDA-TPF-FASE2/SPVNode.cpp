#include "SPVNode.h"

SPVNode::SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("eda_coin/send_merkle_block");
}

void SPVNode::make_response_package(MessageIds id, std::string incoming_address) {
	switch (id) {
	case (MessageIds::MERKLE_BLOCK):
		std::cout << "sending maked package to: " << answers[incoming_address] << std::endl;
		break;
	default:
		std::cout << "Error format in make response: Wrong id type" << std::endl;
		break;
	}
}


void SPVNode::send_request(MessageIds id, string ip, unsigned int port) {
	switch (id) {
	case(MessageIds::TRANSACTION):
		client.methodGet("send_merkle_block", ip, port, 4, 6);
			break;
	case(MessageIds::FILTER):
		break;
	case(MessageIds::GET_BLOCK_HEADER):
		break;
	default:
		std::cout << "Error format in send package: Wrong id type" << std::endl;
		break;
	}
}


SPVNode::~SPVNode() {

}
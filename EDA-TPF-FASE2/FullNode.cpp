#include "FullNode.h"


FullNode::FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("eda_coin/eda_coin/send_block");
	permitedPaths.push_back("eda_coin/send_tx");
	permitedPaths.push_back("eda_coin/send_merkle_block");
	permitedPaths.push_back("eda_coin/send_filter");
	permitedPaths.push_back("eda_coin/get_blocks");
	permitedPaths.push_back("eda_coin/get_block_header");
}

void FullNode::make_response_package(MessageIds id, std::string incoming_address) {
	std::cout << "full node repsonse" << std::endl;
}


void FullNode::send_request(MessageIds id, std::string ip, unsigned int port) {
}


FullNode::~FullNode() {
}
#include "SPVNode.h"

SPVNode::SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/send_merkle_block");

	type = NodeType::SPV;
}

// Funcion que envia un request desde el nodo SPV
void SPVNode::send_request(
	MessageIds id, 
	string ip, 
	unsigned int port, json& Json, unsigned int block_id, unsigned int cant) {

	switch (id) {
	case(MessageIds::TRANSACTION):
		client.methodPost("send_tx", ip, port, Json);
		break;
	case(MessageIds::FILTER):
		client.methodPost("send_filter", ip, port, Json);
		break;
	case(MessageIds::GET_BLOCK_HEADER):
		client.methodGet("get_block_header", ip, port, to_string(block_id), cant);
		break;
	default:
		std::cout << "Error format in send package: Wrong id type" << std::endl;
		break;
	}
}

// Funcion que responde a un pedido desde el nodo SPV
void SPVNode::dispatch_response(string path, string incoming_address, unsigned int block_id, unsigned int count) {
	std::cout << "response_dispatch()" << std::endl;

	json response;

	response["status"] = "true";

	if (path == "/eda_coin/send_merkle_block") {
		response["result"] = "null";
	}
	else {
		std::cout << "NUNCA DEBERIA LLEGAR ACA" << std::endl;
	}

	std::cout << "Respuesta del servidor al pedido:" << endl << response.dump() << std::endl;
	answers[incoming_address] = wrap_package(response.dump());

}

SPVNode::~SPVNode() {

}

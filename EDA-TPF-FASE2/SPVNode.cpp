#include "SPVNode.h"

SPVNode::SPVNode(boost::asio::io_context& io_context, std::string ip, unsigned int port) : GenericNode(io_context, ip, port) {
	// pedidos que permito a mi nodo
	permitedPaths.push_back("/eda_coin/send_merkle_block");
	permitedPaths.push_back("/eda_coin/send_tx");
	currState = NodeState::PASS;
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
void SPVNode::dispatch_response(string path, string incoming_address, json& incoming_json, unsigned int block_id, unsigned int count) {
	std::cout << "response_dispatch()" << std::endl;

	json response;

	response["status"] = "true";

	if (path == "/eda_coin/send_merkle_block") {
		response["result"] = "null";
	}
	else if (path == "/eda_coin/send_tx") {
		response["result"] = "null";
	}

	else {
		std::cout << "NUNCA DEBERIA LLEGAR ACA" << std::endl;
	}

	//std::cout << "Respuesta del servidor al pedido:" << endl << response.dump() << std::endl;
	answers[incoming_address] = wrap_package(response.dump());

}

void SPVNode::doPolls() {
	getNodeIoContext().poll();
	curlPoll();
	static bool b00l = true;
	switch (currState) {
	case NodeState::PASS:
		break;
	case NodeState::APPENDING:
		if (!client.getAnswer().empty()) {
			std::string ping_response_address = neighbour_iterator->first;
			if (client.getAnswer()["status"] == "NETWORK_READY") {
				cout << createAddress(ip, port) << " received NW READY response from " << ping_response_address << endl;
			}
			else {
				cout << createAddress(ip, port) << " received OTHER response from " << ping_response_address << endl;
				remove_address.push_back(ping_response_address);
			}
			neighbour_iterator++;
			client.clearAnswer();
			if (neighbour_iterator == connections.end()) {
				for (string address : remove_address) {
					deleteConnection(address);
				}
				startFiltering();
			}
			else {
				// PING
				json emptyJson;
				emptyJson.clear();
				std::string ping_address = neighbour_iterator->first;
				string ping_ip = get_address_ip(ping_address);
				unsigned int ping_port = get_address_port(ping_address);
				cout << createAddress(ip, port) << " doing ping on " << ping_address << endl;
				client.methodPost("PING", ping_ip.c_str(), ping_port, emptyJson);
			}
		}
		break;
	case NodeState::FILTERING:
		if (!client.getAnswer().empty()) {
			std::string ping_response_address = neighbour_iterator->first;
			cout << createAddress(ip, port) << " received FILTER response from " << ping_response_address << endl;
			neighbour_iterator++;
			client.clearAnswer();
			if (neighbour_iterator == connections.end()) {
				endAppend();
				currState = NodeState::PASS;
			}
			else {
				// FILTER
				json emptyJson;
				emptyJson.clear();
				std::string filter_address = neighbour_iterator->first;
				string filter_ip = get_address_ip(filter_address);
				unsigned int filter_port = get_address_port(filter_address);
				cout << createAddress(ip, port) << " doing filter on " << filter_address << endl;
				// TODO: UN-HARDCODE EMPTY JSON
				client.methodPost("send_filter", filter_ip.c_str(), filter_port, emptyJson);
			}
		}
	}
}

SPVNode::~SPVNode() {

}

void SPVNode::startAppend() {
	neighbour_iterator = connections.begin();
	currState = NodeState::APPENDING;
	// PING
	json emptyJson;
	emptyJson.clear();
	std::string ping_address = neighbour_iterator->first;
	string ping_ip = get_address_ip(ping_address);
	unsigned int ping_port = get_address_port(ping_address);
	cout << createAddress(ip, port) << " doing ping on " << ping_address << endl;
	client.methodPost("PING", ping_ip.c_str(), ping_port, emptyJson);
}

void SPVNode::endAppend(){
	neighbour_iterator = connections.begin();
	if (connections.size() == 0) {
		cout << createAddress(ip, port) << "has no connections!" << endl;
		return;
	}
	unsigned int node_to_connect = rand() % connections.size() + 0;
	for (unsigned int i = 0; i < node_to_connect; i++) {
		neighbour_iterator++;
	}

	string node_to_connect_addres = neighbour_iterator->first;

	cout << createAddress(ip, port) << " is asking " << node_to_connect_addres << " for headers..." << endl;
	// TODO: UN HARDCODE get_block_header parameters
	string headers_ip = get_address_ip(node_to_connect_addres);
	unsigned int headers_port = get_address_port(node_to_connect_addres);
	client.methodGet("get_block_header", headers_ip, headers_port, "00000000", 33);
	neighbour_iterator = connections.begin();
}

void SPVNode::startFiltering() {
	currState = NodeState::FILTERING;
	neighbour_iterator = connections.begin();
	client.clearAnswer();
	json emptyJson;
	emptyJson["Key"] = createAddress(ip, port);
	emptyJson.clear();
	std::string filter_address = neighbour_iterator->first;
	string filter_ip = get_address_ip(filter_address);
	unsigned int filter_port = get_address_port(filter_address);
	cout << createAddress(ip, port) << " doing filter on " << filter_address << endl;
	client.methodPost("send_filter", filter_ip.c_str(), filter_port, emptyJson);
}

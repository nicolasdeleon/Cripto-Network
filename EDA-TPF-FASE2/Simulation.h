#pragma once
#include "SPVNode.h"
#include "FullNode.h"
#include <string>
#include <iostream>
#include <vector>

#define REQUEST_BUFFER_LENGTH 100000
#define AMOUNT_OF_PATHS 3

using namespace std;
enum class NodeType { FULL, SPV };

class Simulation {
public:
	Simulation();
	bool addNode(string ip, unsigned int port, NodeType NodeType);
	void addNodeAndStart(string ip, unsigned int port, NodeType NodeType);
	void sendMessageFromNode2Node(
		string ip_origen, 
		unsigned int port_origen, 
		string ip_destino, 
		unsigned int port_destino,
		MessageIds request_id);
	bool deleteNode(string ip, unsigned int port);
	bool createConnection(string ip_origen, int puerto_origen, string ip_destino, int puerto_destino);
	bool deleteConnection(string origin_adress, string destiny_address);
	void startNodes();
	void doNodePolls();
	vector<GenericNode*> getNodes();
	~Simulation();
private:
	string createAddress(string ip, int port);

	vector<GenericNode*> Nodes;
	vector<boost::asio::io_context*> contexts;
};



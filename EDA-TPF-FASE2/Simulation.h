#pragma once
#include "SPVNode.h"
#include "FullNode.h"
#include <string>
#include <iostream>
#include <vector>


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
		MessageIds request_id, unsigned int block_id = 0, unsigned int cant = 0, json Json = {});
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



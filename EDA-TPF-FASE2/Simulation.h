#pragma once
#include "SPVNode.h"
#include "FullNode.h"
#include "MinerNode.h"
#include <string>
#include <iostream>
#include <vector>


using namespace std;


class Simulation {
public:
	Simulation();
	GenericNode* addNode(string ip, unsigned int port, NodeType NodeType);
	void addNodeAndStart(string ip, unsigned int port, NodeType NodeType);
	void sendMessageFromNode2Node(
	string ip_origen, 
	unsigned int port_origen, 
	string ip_destino, 
	unsigned int port_destino,
	MessageIds request_id, json& Json, unsigned int block_id, unsigned int cant);
	bool deleteNode(string ip, unsigned int port);
	void sendTransaction(string origin_adress, vector<string>& target_adresses, vector<int>& amounts);
	bool createConnection(string ip_origen, int puerto_origen, string ip_destino, int puerto_destino);
	bool deleteConnection(string origin_adress, string destiny_address);
	void startNodes();
	void doNodePolls();
	bool appendNode(string& my_ip, unsigned int& my_port, NodeType my_type, vector<string>& neighborhood);
	vector<GenericNode*> getNodes();
	string getRequestAnswer(string address);
	vector<GenericNode*> Nodes;
	void giveAddress2Nodes(vector<string>& addss);
	bool areFullReady();
	void connectSPV();
	vector<unsigned int> spvGenNodes;

	
	~Simulation();
private:
	string createAddress(string ip, int port);
	vector<boost::asio::io_context*> contexts;


};



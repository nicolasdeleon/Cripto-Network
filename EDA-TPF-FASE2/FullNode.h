#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>

#include <allegro5/allegro.h>

enum class NodeState { IDLE, COL_NETW_MEMBS, WAITING_NETW_LAYOUT, NETW_CREATED, WAITING_PING_RESPONSE, SENDING_LAYOUTS, WAITING_LAYOUT_RESPONSE};
enum class NodeEvents { TIMEOUT, NETW_NOT_READY, NETW_READY_PING, PING};


class FullNode : public GenericNode
{
public:
	FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	~FullNode();
	void send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant);
	void sendTX(std::string path, std::string outIp, int outPort, vector<int> amounts, vector<std::string> publicIds);
	void algoritmoParticular();
	void doPolls();
	
	
	
private:
	
	void executeLayout();
	vector<NodeInfo> pingedNodes;
	void pingNodes();
	NodeState currState;
	bool es_conexo(void);
	void sendMklBlock(std::string path, std::string outIp, int outPort, std::string blockId, int tx_pos);
	void sendFilter(std::string path, std::string outIp, int outPort);
	void sendBlock(std::string path_, std::string outIp, int outPort, std::string blockId);
	void getBlocks(std::string path_, std::string outIp, int outPort, std::string blockId, int numBlocks);
	void dispatch_response(std::string path, std::string incoming_address, unsigned int block_id = 0, unsigned int count = 0);
	std::string hexCodexASCII(unsigned int number);
	unsigned int generateID(unsigned char* str);
	vector<std::string> makeMerklePath(int blockNumber, std::string txid);
	json to_send;
	json layout;
	ALLEGRO_TIMER* timer;
	ALLEGRO_EVENT_QUEUE* queue;
	string pingingNodeAdress;
	int countdown;
};





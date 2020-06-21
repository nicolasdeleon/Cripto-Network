#pragma once
#include "GenericNode.h"
#include <string>
#include <iostream>
#include <allegro5/allegro.h>
#include <queue>
#include "TXfloodRequest.h"
#include "BLKfloodRequest.h"



class FullNode : public GenericNode
{
	enum class NodeState {
		IDLE,
		COL_NETW_MEMBS,
		WAITING_NETW_LAYOUT,
		NETW_CREATED,
		WAITING_PING_RESPONSE,
		SENDING_LAYOUTS,
		WAITING_LAYOUT_RESPONSE,
		APPENDING,
		FLOOD,
		WAITING_FLOOD_RESPONSE,
		FLOOD_BLOCK,
		WAITING_BLKFLOOD_RESPONSE,
	};

public:

	FullNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
	~FullNode();

	void doPolls();
	
	bool getState() {
		if (currState == NodeState::NETW_CREATED) {
			return true;
		}
		else {
			return false;
		}
	};
	json getBlockChain_FULL(void);
	void goFlood(vector<string>&pubids, vector<int>&amount);

private:

	// Node State
	
	NodeState currState;

	//flood (tsx)

	void flood_transaction();
	void flood_block();
	bool parseIncoming(json incoming_json);
	vector<std::string> mensajesRecibidos;
	queue<TXfloodRequest> pendingFloodRequests;
	queue<BLKfloodRequest> pendingBlockFloodRequests;
	void sendTX(std::string path, std::string outIp, int outPort, vector<int> amounts, vector<std::string> publicIds);

	//setup

	void pingNodes();
	void executeLayout();
	bool es_conexo(void);
	void algoritmoParticular();
	ALLEGRO_TIMER* timer;
	ALLEGRO_EVENT_QUEUE* AlEvQueue;
	int countdown;
	string pingingNodeAdress;
	vector<NodeInfo> pingedNodes;
	json layout;

	void startAppend();
	void endAppend();
	

	// send/get stuff

	void sendMklBlock(std::string path, std::string outIp, int outPort, std::string blockId, int tx_pos);
	vector<std::string> makeMerklePath(int blockNumber, std::string txid);
	std::string hexCodexASCII(unsigned int number);
	unsigned int generateID(unsigned char* str);
	void sendFilter(std::string path, std::string outIp, int outPort);
	void sendBlock(std::string path_, std::string outIp, int outPort, std::string blockId);
	void getBlocks(std::string path_, std::string outIp, int outPort, std::string blockId, int numBlocks);
	
	void dispatch_response(std::string path, std::string incoming_address, json& incoming_json, unsigned int block_id = 0, unsigned int count = 0);


	void send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id, unsigned int cant);

};





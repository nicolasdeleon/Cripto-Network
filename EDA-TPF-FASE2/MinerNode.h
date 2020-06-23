#pragma once

#include "FullNode.h"

class MinerNode : public FullNode {
	public:

		MinerNode(boost::asio::io_context& io_context, std::string ip, unsigned int port);
		~MinerNode();
		void doPolls();
	
	protected:
		void mineBlock();
		json createMiningFee();
		void processTsx(string& inc_adrs, json& inc_json);
		vector<json> nextBlockTSXs;
};
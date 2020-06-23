#pragma once
#include <boost\asio.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include "MyClient.h"
#include "blockchainHandler.h"
#define REMOVE "REMOVE"
#define REQUEST_BUFFER_LENGTH 700
#define AMOUNT_OF_PATHS 10
#define TIME_OUT 5000


enum class NodeType { FULL, SPV, MINER };

typedef struct nodeInfo {
	string ip;
	int puerto;
	int connections = 0;
	bool visitado = false;
	vector<int> conectedWith;
} NodeInfo;

using message_id = unsigned int;
enum MessageIds : message_id { MERKLE_BLOCK, BLOCK, TRANSACTION, GET_BLOCK_HEADER, GET_BLOCKS, FILTER, PING };

class GenericNode {
public:
	GenericNode(boost::asio::io_context& io_context, std::string ip = "127.0.0.1", unsigned int port = 80);
	virtual ~GenericNode();
	void start();
	std::map<std::string, boost::asio::ip::tcp::socket*> getConnections();
	void addConnection(std::string destiny_address);
	bool deleteConnection(std::string destiny_address);
	unsigned int getPort();
	std::string getIP();
	std::string getAddress();
	boost::asio::io_context& getNodeIoContext();
	virtual void send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id = 0, unsigned int cant = 0)=0;
	void setPort(unsigned int PORT = 80);
	void curlPoll();
	string getClientRequestAnswer();
	NodeType getType();
	//virtual void algoritmoParticular(void);
	virtual void doPolls() = 0;
	void giveAvailableNodes(vector<string>& genesisNodes);
	virtual void startAppend() = 0;
	virtual void endAppend() = 0;
	virtual bool getState() = 0;
	string getstr(void);
	json getBlockChain(void);

protected:

	std::vector<std::string> extract_keys(std::map<std::string, boost::asio::ip::tcp::socket*> const& input_map);
	vector<string> genesisNodes;
	std::string createAddress(std::string ip, int port);
	void listen_connection();
	void answer(std::string incoming_address);
	void connection_received_cb(const boost::system::error_code& error);
	void response_sent_cb(const boost::system::error_code& error, size_t bytes_sent, std::string incoming_address);
	void read(std::string incoming_address);
	void message_received_cb(const boost::system::error_code& error, size_t bytes_sent, std::string incoming_address);
	bool parse_request(std::string incoming_address);
	void shutdown_open_sockets();
	void shut_down_reciever_socket();
	void shutdown_socket_for_connection(std::string incoming_address);
	virtual void dispatch_response(string path, string incoming_address, json& incoming_json, unsigned int block_id = 0, unsigned int count = 0) = 0;
	string parseEndPoint(char* str);
	unsigned int parseBlockId(char* str);
	unsigned int parseCount(char* str);
	NodeType type;
	string get_address_ip(string& address);
	string str;
	unsigned int get_address_port(string& address);
	json incoming_json;

	std::string wrap_package(string json_string);

	std::map<std::string, std::vector<unsigned char>> requests;
	
	boost::asio::io_context& context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket* handler_socket;
	unsigned int port;
	std::string ip;
	std::map<std::string, boost::asio::ip::tcp::socket*> connections;
	
	std::vector<std::string> permitedPaths;
	std::map<std::string, std::string> answers;
	MyClient client;
	vector<string> keys_list;
	json blockChain;
	blockchainHandler blockchainHandler;
	std::map<string, boost::asio::ip::tcp::socket*>::iterator neighbour_iterator;
	vector<string> remove_address;

};



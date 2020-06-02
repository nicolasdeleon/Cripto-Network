#pragma once
#pragma once
#include <boost\asio.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include "MyClient.h"

#define REQUEST_BUFFER_LENGTH 500
#define AMOUNT_OF_PATHS 3

using message_id = unsigned int;
enum MessageIds : message_id { MERKLE_BLOCK, BLOCK, TRANSACTION, GET_BLOCK_HEADER, GET_BLOCKS, FILTER };

class GenericNode {
public:
	GenericNode(boost::asio::io_context& io_context, std::string ip = "127.0.0.1", unsigned int port = 80);
	~GenericNode();
	void start();
	std::map<std::string, boost::asio::ip::tcp::socket*> getConnections();
	void addConnection(std::string destiny_address);
	bool deleteConnection(std::string destiny_address);
	unsigned int getPort();
	std::string getIP();
	std::string getAddress();
	boost::asio::io_context& getNodeIoContext();
	virtual void send_request(MessageIds id, std::string ip, unsigned int port) = 0;
	void setPort(unsigned int PORT = 80);


protected:
	virtual void make_response_package(MessageIds id, std::string incoming_address) = 0;
	MyClient client;
	std::vector<std::string> permitedPaths;
	std::map<std::string, std::string> answers;

private:
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

	std::string wrap_package(std::string incoming_address);

	std::string html_requested;
	std::map<std::string, std::vector<unsigned char>> requests;
	
	boost::asio::io_context& context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket* handler_socket;
	unsigned int port;
	std::string ip;
	std::string address;
	std::map<std::string, boost::asio::ip::tcp::socket*> connections;

};



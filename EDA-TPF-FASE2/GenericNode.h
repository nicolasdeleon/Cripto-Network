#pragma once
#include <boost\asio.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>
#include "MyClient.h"

#define REQUEST_BUFFER_LENGTH 700
#define AMOUNT_OF_PATHS 5

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
	virtual void send_request(MessageIds id, std::string ip, unsigned int port, json& Json, unsigned int block_id = 0, unsigned int cant = 0) = 0;
	void setPort(unsigned int PORT = 80);
	void curlPoll();
	

protected:
	std::vector<std::string> permitedPaths;
	std::map<std::string, std::string> answers;
	MyClient client;

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
	void dispatch(string path, string incoming_address, unsigned int block_id = 0, unsigned int count = 0);
	void parseHtmlRequested();

	std::string wrap_package(string json_string);

	std::string html_requested;
	std::map<std::string, std::vector<unsigned char>> requests;
	
	boost::asio::io_context& context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket* handler_socket;
	unsigned int port;
	std::string ip;
	std::string address;
	std::map<std::string, boost::asio::ip::tcp::socket*> connections;
	std::string counter, block_id, path;
};



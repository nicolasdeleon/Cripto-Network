#pragma once
#include <boost\asio.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <vector>

#define REQUEST_BUFFER_LENGTH 100000
#define AMOUNT_OF_PATHS 3

using namespace std;

class GenericNode {
public:
	GenericNode(boost::asio::io_context& io_context, unsigned int port = 80);
	~GenericNode();
	void startNode();
	std::map<unsigned int, boost::asio::ip::tcp::socket*> getConnections();
	unsigned int getPort();
	boost::asio::io_context& getNodeIoContext();

	std::vector<std::string> permitedPaths;

	void setPort(unsigned int PORT = 80);

private:
	void listen_connection();
	void answer(unsigned int port);
	void connection_received_cb(const boost::system::error_code& error);
	void response_sent_cb(const boost::system::error_code& error, size_t bytes_sent, unsigned int port);
	void read(unsigned int port);
	void message_received_cb(const boost::system::error_code& error, size_t bytes_sent, unsigned int port);
	bool parse_request();
	void shutdown_open_sockets();
	void shut_down_reciever_socket();
	void shutdown_socket_for_connection(unsigned int port);

	std::string make_package();

	std::string msg;
	std::string html_requested;
	std::vector<unsigned char> request;

	boost::asio::io_context& context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket* handler_socket;
	unsigned int port;
	// cada nodo guarda sus conexiones con otros nodos
	std::map<unsigned int, boost::asio::ip::tcp::socket*> connections;
};

class Simulation {
public:
	Simulation();
	void addNode(unsigned int port);
	void startNodes();
	void doNodePolls();
	~Simulation();
private:
	vector<GenericNode*> Nodes;
	vector<boost::asio::io_context*> contexts;
};

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
	GenericNode(boost::asio::io_context& io_context, string ip = "127.0.0.1", unsigned int port = 80);
	~GenericNode();
	void start();
	std::map<string, boost::asio::ip::tcp::socket*> getConnections();
	void addConnection(string destiny_address);
	bool deleteConnection(string destiny_address);
	unsigned int getPort();
	std::string getIP();
	std::string getAddress();
	boost::asio::io_context& getNodeIoContext();

	std::vector<std::string> permitedPaths;

	void setPort(unsigned int PORT = 80);


private:
	string createAddress(string ip, int port);
	void listen_connection();
	void answer(string incoming_address);
	void connection_received_cb(const boost::system::error_code& error);
	void response_sent_cb(const boost::system::error_code& error, size_t bytes_sent, string incoming_address);
	void read(string incoming_address);
	void message_received_cb(const boost::system::error_code& error, size_t bytes_sent, string incoming_address);
	bool parse_request();
	void shutdown_open_sockets();
	void shut_down_reciever_socket();
	void shutdown_socket_for_connection(string incoming_address);

	std::string make_package();

	// myClient client;

	std::string msg;
	std::string html_requested;
	std::vector<unsigned char> request;

	boost::asio::io_context& context_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::asio::ip::tcp::socket* handler_socket;
	unsigned int port;
	string ip;
	string address;
	//string IP = POR AHORA HARDOCEADO A LOCAL HOST 127.0.0.1;
	// cada nodo guarda sus conexiones con otros nodos
	std::map<string, boost::asio::ip::tcp::socket*> connections;
	// connections:
	// "IP/puerto"-> 
	// "IP/puerto2"->
};



class Simulation {
public:
	Simulation();
	void addNode(string ip, unsigned int port);
	bool deleteNode(string ip, unsigned int port);
	bool createConnection(string ip_origen, int puerto_origen, string ip_destino, int puerto_destino);
	bool deleteConnection(string ip_origen, int puerto_origen, string ip_destino, int puerto_destino);
	void startNodes();
	void doNodePolls();
	// void connect2Nodes();
	~Simulation();
private:
	string createAddress(string ip, int port);

	vector<GenericNode*> Nodes;
	vector<boost::asio::io_context*> contexts;
};

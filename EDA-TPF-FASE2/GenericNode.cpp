#include "GenericNode.h"

using namespace std::placeholders;

using namespace std;

using boost::asio::ip::tcp;


string GenericNode::createAddress(string ip, int port) {
	string address = ip + ":" + to_string(port);
	return address;
}


GenericNode::GenericNode(boost::asio::io_context& io_context, string ip, unsigned int port)
	: context_(io_context),
	acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
	port(port),
	ip(ip),
	client(ip, port+1),
	address(createAddress(ip, port)),
	handler_socket(nullptr),
	permitedPaths(AMOUNT_OF_PATHS)
{

}

GenericNode::~GenericNode()
{

	// deberia destruir el mapa de sockets, de todas formas es probable que ya este destruido porque destruyo cada socket cada vez q cierro la conexion
}

std::map<string, boost::asio::ip::tcp::socket*>
GenericNode::getConnections() {
	return connections;
}

void GenericNode::addConnection(string destiny_address) {
	connections.insert(std::pair<string, boost::asio::ip::tcp::socket*>(destiny_address, nullptr));
	requests.insert(std::pair<string, vector<unsigned char>>(destiny_address, vector<unsigned char>(0)));
	answers.insert(std::pair<string, string>(destiny_address, ""));
}

bool GenericNode::deleteConnection(string destiny_address) {
	if (connections[destiny_address] != nullptr)
		delete(connections[destiny_address]);
	bool res = connections.erase(destiny_address);
	return res;
}

unsigned int GenericNode::getPort() {
	return port;
}

string GenericNode::getIP() {
	return ip;
}

string GenericNode::getAddress() {
	return address;
}

void GenericNode::setPort(unsigned int PORT) {
	acceptor_ = boost::asio::ip::tcp::acceptor(context_, tcp::endpoint(tcp::v4(), PORT));
	port = PORT;
}

boost::asio::io_context& GenericNode::getNodeIoContext() {
	return context_;
}

void GenericNode::shutdown_open_sockets() {

	std::map<string, boost::asio::ip::tcp::socket*>::iterator iterator = connections.begin();
	while (iterator != connections.end())
	{
		// Accessing KEY from element pointed by it.
		//std::string word = it->first;
		boost::asio::ip::tcp::socket* socket = iterator->second;

		if (socket != nullptr && socket->is_open()) {
			socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
			socket->close();
		}
		iterator->second = nullptr;
		iterator++;
	}
}


void GenericNode::start() {
	// apago y cierro el socket recien creado, REDUNDANTE, esto se podria sacar pero me tiro error si lo saco
	// es el estado inicial de cada nodo
	shutdown_open_sockets();
	listen_connection();
}


void GenericNode::listen_connection()
{
	std::cout << "listening conection..." << std::endl;
	this->handler_socket = new boost::asio::ip::tcp::socket(context_);

	acceptor_.async_accept(
		*(this->handler_socket),
		[this](const boost::system::error_code& error) {
			GenericNode::connection_received_cb(error);
		});
}


void GenericNode::shut_down_reciever_socket() {
	(*handler_socket).shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	(*handler_socket).close();
	delete(handler_socket);
}

void GenericNode::shutdown_socket_for_connection(string incoming_address) {
	(*(connections[incoming_address])).shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	(*(connections[incoming_address])).close();
	delete(connections[incoming_address]);
	connections[incoming_address] = nullptr;
}

void GenericNode::connection_received_cb(const boost::system::error_code& error)
{
	std::cout << "connection_received_cb()" << std::endl;
	// me fijo de donde vino la conexion
	string ip = handler_socket->remote_endpoint().address().to_string();
	unsigned int port = handler_socket->remote_endpoint().port();
	std::cout << ip << " ";
	std::cout << port << std::endl;
	address = createAddress(ip, port-1);
	std::cout << address << endl;

	// Checkeo que la conexion sea de un puerto de mi red
	if (connections.find(address) != connections.end()) {
		// creo un socket y lo coloco en mis connecciones, expandoe l tamaño de mi vector de request
		boost::asio::ip::tcp::socket* socket = handler_socket;
		connections[address] = socket;
		requests[address].resize(REQUEST_BUFFER_LENGTH);
	}
	else {
		cout << "port no pertenece a la red" << endl;
		shut_down_reciever_socket(); // para poder recibir otras conexiones
		listen_connection();
		return;
	}

	listen_connection();

	if (!error) {
		read(address);
	}
	else {
		std::cout << error.message() << std::endl;
	}
}


void GenericNode::answer(string incoming_address)
{
	std::cout << "answer()" << std::endl;
	if (answers[incoming_address] != "") {

		cout << "Estoy mandando esto: " << endl;
		cout << answers[incoming_address] << endl;
		boost::asio::async_write(
			*(connections[incoming_address]),
			boost::asio::buffer(answers[incoming_address]),
			[this, incoming_address](const boost::system::error_code& error, size_t bytes_sent) {
				this->response_sent_cb(error, bytes_sent, incoming_address);
			}
		);
	}
	else {
		cout << "Nada cargado en answer" << " // " << "Pedido de " << incoming_address << " a nodo " << address;
		shutdown_socket_for_connection(incoming_address);
		listen_connection();
	}
}


void GenericNode::response_sent_cb(const boost::system::error_code& error,
	size_t bytes_sent, string incoming_address)
{
	std::cout << "response_sent_cb()" << std::endl;
	if (!error) {
		std::cout << "Response sent. " << bytes_sent << " bytes." << std::endl;
	}
	shutdown_socket_for_connection(incoming_address);
	listen_connection();
}


void GenericNode::read(string incoming_address) {
	std::cout << "read()" << std::endl;
	// TODO: REMOVE THIS DEBUG IF
	if (connections[incoming_address] == nullptr) {
		cout << "handler is null" << endl;
	}
	(*(connections[incoming_address])).async_receive(
		boost::asio::buffer(requests[incoming_address].data(), requests[incoming_address].size()),
		std::bind(&GenericNode::message_received_cb, this, std::placeholders::_1, std::placeholders::_2, incoming_address)
	);
}


bool GenericNode::parse_request(string incoming_address) {
	std::cout << "parse_request()" << std::endl;

	bool ret = false;
	unsigned int block_id, count;
	string path_requested;

	std::string mystring(requests[incoming_address].begin(), requests[incoming_address].end());

	// CHECK: 
		// - http v1.1 ?
		// - GET or POST ?
	// RETRIEVE:
		// - path_requested

	if (mystring.size() != 0
		&& (mystring.find("GET") == 0 || mystring.find("POST") == 0)
		&& mystring.find("HTTP/1.1") != mystring.npos)
	{
		// recibi un get
		std::size_t reference_size = mystring.find("HTTP/1.1") - 5;
		html_requested = mystring.substr(5, reference_size - 1);
		std::cout << "Parsed without errors. The requested html is " << html_requested << std::endl;
		
		// TODO: obtengo block id y count de html_requested
		// html_requested = ....
		// block_id = ...
		// count
		
		ret = true;
	}
	else {
		json response;
		response["status"] = false;
		response["result"] = 1;
		answers[incoming_address] = wrap_package(response.dump());
		std::cout << "Request error " << std::endl;
		html_requested = "error";
	}

	// CHECK:
		// Node can handle request ?
	if (std::find(permitedPaths.begin(), permitedPaths.end(), html_requested) != permitedPaths.end())
	{
		// dispatch(html_requested, incoming_address, block_id, count);
		cout << "ACA HAY QUE DESCOMENTAR DISPATCH" << endl;
	}
	else {

		json response;
		response["status"] = false;
		response["result"] = 1;
		answers[incoming_address] = wrap_package(response.dump());
	
	}

	return ret;
}


void GenericNode::dispatch(string path, string incoming_address, unsigned int block_id
	, unsigned int count) {
	
	json response;
	
	if (path == "eda_coin/send_block") {

		// response = make_send_block_json

	}
	else if (path == "eda_coin/send_tx") {

		// response = make_send_block_json
	}
	else if (path == "eda_coin/send_merkle_block") {

		// response = make_send_block_json
	}
	else if (path == "eda_coin/send_filter") {

		// response = make_send_block_json
	}
	else if (path == "eda_coin/get_blocks") {

		// response = make_send_block_json
	}
	else if (path == "eda_coin/get_block_header") {

		// response = make_send_block_json
	}
	else {
		std::cout << "NUNCA DEBERIA LLEGAR ACA" << std::endl;
	}

	answers[incoming_address] = wrap_package(response.dump());
}


void GenericNode::message_received_cb(const boost::system::error_code& error, size_t bytes_sent, string incoming_address) {

	std::cout << "message_received_cb()" << std::endl;
	if (!error) {
		std::cout << "Response correctly received. " << bytes_sent << " bytes." << std::endl;
	}
	else {
		std::cout << bytes_sent << " bytes." << std::endl;
	}

	// DEBUG INFO RECIEVED
	string decode;
	for (int i = 0; i < requests[incoming_address].size(); i++) {
		std::cout << requests[incoming_address][i];
		if(requests[incoming_address][i] != '\0')
			decode += requests[incoming_address][i];
	}

	parse_request(incoming_address);
	answer(incoming_address);
}


std::string GenericNode::wrap_package(string json_string)
{
	#pragma warning(disable : 4996)
	using namespace std; // For time_t, time and ctime;
	string pkg = "";


	//(https://stackoverflow.com/questions/997512/string-representation-of-time-t)//
	std::time_t now = std::time(NULL);
	std::tm* ptm = std::gmtime(&now);
	char time1[32];
	std::strftime(time1, 32, "%a, %d %b %Y %H:%M:%S GMT", ptm);

	std::time_t now_and_30secs = std::time(NULL) + 30;
	ptm = std::gmtime(&now_and_30secs);
	char time2[32];
	std::strftime(time2, 32, "%a, %d.%m.%Y %H:%M:%S GMT", ptm);
	//----------------------------------------------------------------------------//

	int file_length = json_string.size();
	pkg = "HTTP/1.1 200 OK\r\n";
	pkg += "Date: " + (string)time1 + "\r\n";
	pkg += "Location: 127.0.0.1:80\r\n";
	pkg += "Cache-Control: max-age=30\r\n";
	pkg += "Expires: " + (string)time2 + "\r\n";
	pkg += "Content-Length: " + to_string(file_length) + "\r\n";
	pkg += "Content-Type: application/json; charset=iso-8859-1\r\n";
	pkg += "\r\n";
	pkg += json_string;
	pkg += "\r\n";
	
	/*
	else {
		pkg += "HTTP/1.1 404 Not Found\r\n";
		pkg += "Date: " + (string)time1 + "\r\n";
		pkg += "Cache-Control: public, max-age=30\r\n";
		pkg += "Expires: " + (string)time2 + "\r\n";
		pkg += "Content-Length: 0\r\n";
		pkg += "\r\n";
		pkg += "\r\n";
	}
	*/

	return pkg;
}

void GenericNode::curlPoll() {
	client.performRequest();
}


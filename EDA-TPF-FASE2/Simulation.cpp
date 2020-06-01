#include "Simulation.h"

using namespace std::placeholders;

using boost::asio::ip::tcp;


GenericNode::GenericNode(boost::asio::io_context& io_context, unsigned int port)
	: context_(io_context),
	acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
	port(port),
	handler_socket(nullptr),
	request(REQUEST_BUFFER_LENGTH),
	permitedPaths(AMOUNT_OF_PATHS)
{
 	permitedPaths.push_back("randompath/radioaficionados.html");
	permitedPaths.push_back("index.html");
	permitedPaths.push_back("randompath/elviajesito.html");
}

GenericNode::~GenericNode() 
{

	// deberia destruir el mapa de sockets, de todas formas es probable que ya este destruido porque destruyo cada socket cada vez q cierro la conexion
}

std::map<unsigned int, boost::asio::ip::tcp::socket*> 
GenericNode::getConnections() {
	return connections;
}

unsigned int GenericNode::getPort() {
	return port;
}

void GenericNode::setPort(unsigned int PORT) {
	acceptor_ = boost::asio::ip::tcp::acceptor(context_, tcp::endpoint(tcp::v4(), PORT));
}

boost::asio::io_context& GenericNode::getNodeIoContext() {
	return context_;
}

void GenericNode::shutdown_open_sockets() {

	std::map<unsigned int, boost::asio::ip::tcp::socket*>::iterator iterator = connections.begin();
	while (iterator != connections.end())
	{
		// Accessing KEY from element pointed by it.
		//std::string word = it->first;
		boost::asio::ip::tcp::socket* socket = iterator->second;
		if (socket->is_open()) {
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

void GenericNode::shutdown_socket_for_connection(unsigned int port) {
	(*(connections[port])).shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	(*(connections[port])).close();
	delete(connections[port]);
	connections[port] = nullptr;
}

void GenericNode::connection_received_cb(const boost::system::error_code& error)
{
	std::cout << "connection_received_cb()" << std::endl;
	// me fijo de donde vino la conexion
	string address = handler_socket->remote_endpoint().address().to_string();
	unsigned int port = handler_socket->remote_endpoint().port();
	std::cout << address << " ";
	std::cout << port << std::endl;

	// Checkeo que la conexion sea de un puerto de mi red
	if (connections.find(port) != connections.end()) {
		// creo un socket y lo coloco en mis connecciones
		boost::asio::ip::tcp::socket* socket = handler_socket;
		connections[port] = socket;
	}
	else {
		cout << "port no pertenece a la red" << endl;
		//		cout << "agrego a la red esta conexion para poder debuguear el proceso de rta" << endl;
		shut_down_reciever_socket(); // para poder recibir otras conexiones
		listen_connection();
		return;
	}

	listen_connection();

	if (!error) {
		read(port);
	}
	else {
		std::cout << error.message() << std::endl;
	}
}


void GenericNode::answer(unsigned int port)
{
	std::cout << "answer()" << std::endl;
	if (request.size() != 0) {
		// OBTENGO INFO A MANDAR
		// TODO: crear mapa de info a mandar <nodo, info>
		// ESTAMOS EN NODO GENERICO
		// <nodo1, info_a_mandar> 
		// <nodo2, info_a_mandar> 
		// <nodo3, info_a_mandar>
		msg = make_package();
		boost::asio::async_write(
			*(connections[port]),
			boost::asio::buffer(msg),
			[this, port](const boost::system::error_code& error, size_t bytes_sent) {
				this->response_sent_cb(error, bytes_sent, port);
			}
		);
	}
}


void GenericNode::response_sent_cb(const boost::system::error_code& error,
	size_t bytes_sent, unsigned int port)
{
	std::cout << "response_sent_cb()" << std::endl;
	if (!error) {
		std::cout << "Response sent. " << bytes_sent << " bytes." << std::endl;
	}
	shutdown_socket_for_connection(port);
	listen_connection();
}


void GenericNode::read(unsigned int port) {
	std::cout << "read()" << std::endl;
	if (connections[port] == nullptr) {
		cout << "handler is null" << endl;
	}
	(*(connections[port])).async_receive(
		// TODO: crear mapa de info <nodo, list()>
		// ESTAMOS EN NODO GENERICO
		// <nodo1, info_que_me_llego> 
		// <nodo2, info_que_me_llego> 
		// <nodo3, info_que_me_llego>
		boost::asio::buffer(request.data(), request.size()),
		std::bind(&GenericNode::message_received_cb, this, _1, _2, port)
	);
}


bool GenericNode::parse_request() {
	std::cout << "parse_request()" << std::endl;

	// TODO: RE HACER TODO EL PARSER !!!!!
	// 

	bool ret = false;

	// OBTENGO INFO DEL NODO QUE ME HABLO
	// std::string infoAParsar(ACA DEBERIA METER LA INFO DEL NODO QUE ME HABLO);

	// ARMO MENSAJE

	// TODO: crear mapa de info a mandar <nodo, info>
	// ESTAMOS EN NODO GENERICO
	// <nodo1, info_a_mandar> 
	// <nodo2, info_a_mandar> 
	// <nodo3, info_a_mandar>

	std::string mystring(request.begin(), request.end());

	// check if it is a get http(v1.1) request and that we are the right target
	if (mystring.size() != 0
		&& mystring.find("GET") == 0
		&& mystring.find("Host: 127.0.0.1") != mystring.npos
		&& mystring.find("HTTP/1.1") != mystring.npos)
	{
		std::size_t reference_size = mystring.find("HTTP/1.1") - 5;
		html_requested = mystring.substr(5, reference_size - 1);
		std::cout << "Parsed without errors. The requested html is " << html_requested << std::endl;
		ret = true;
	}
	else {
		std::cout << "Request error " << std::endl;
		html_requested = "error";
	}
	return ret;
}


void GenericNode::message_received_cb(const boost::system::error_code& error, size_t bytes_sent, unsigned int port) {

	std::cout << "message_received_cb()" << std::endl;
	if (!error) {
		std::cout << "Response correctly received. " << bytes_sent << " bytes." << std::endl;
	}
	else {
		std::cout << bytes_sent << " bytes." << std::endl;
	}

	//print(request); to use this print function uncomment top of file print(...)
	parse_request();
	answer(port);
}


std::string GenericNode::make_package()
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


	string enter = "";
	bool availablePath = false;
	cout << "make_package()" << endl;
	cout << "Path requested: " << html_requested << endl;
	for (int i = 0; i < permitedPaths.size(); i++) {
		if (html_requested == permitedPaths.at(i)) {
			availablePath = true;
		}
		cout << "Path available: " << permitedPaths.at(i) << endl;
	}
	cout << "availablePath ?  " << availablePath << endl;
	if (availablePath) {
		std::ifstream t(html_requested);
		std::string html_to_send((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		int file_length = html_to_send.size();
		pkg = "HTTP/1.1 200 OK\r\n";
		pkg += "Date: " + (string)time1 + "\r\n";
		pkg += "Location: 127.0.0.1/data.html\r\n";
		pkg += "Cache-Control: max-age=30\r\n";
		pkg += "Expires: " + (string)time2 + "\r\n";
		pkg += "Content-Length: " + to_string(file_length) + "\r\n";
		pkg += "Content-Type: text/html; charset=iso-8859-1\r\n";
		pkg += "\r\n";
		pkg += html_to_send;
		pkg += "\r\n";
	}
	else {
		pkg += "HTTP/1.1 404 Not Found\r\n";
		pkg += "Date: " + (string)time1 + "\r\n";
		pkg += "Cache-Control: public, max-age=30\r\n";
		pkg += "Expires: " + (string)time2 + "\r\n";
		pkg += "Content-Length: 0\r\n";
		pkg += "\r\n";
		pkg += "\r\n";
	}

	cout << pkg.c_str() << endl;

	return pkg;
}



// ---------------------------------------------- SIMULATION STUFF ---------------------------------------------
Simulation::Simulation()
{

}


Simulation::~Simulation() {
	for (GenericNode* node : Nodes) {
		delete(node);
	}
	for (boost::asio::io_context* context : contexts) {
		delete(context);
	}
}


void Simulation::addNode(unsigned int port) {

	boost::asio::io_context* context = new boost::asio::io_context();
	contexts.push_back(context);
	GenericNode* newNode = new GenericNode(*context, port);
	for (GenericNode* node : Nodes) {
		newNode->getConnections().insert(std::pair<unsigned int, boost::asio::ip::tcp::socket*>(node->getPort(), nullptr));
		node->getConnections().insert(std::pair<unsigned int, boost::asio::ip::tcp::socket*>(newNode->getPort(), nullptr));
	}
	Nodes.push_back(newNode);
}


void Simulation::startNodes() {
	for (GenericNode* node : Nodes) {
		// Itero por todos mis nodos y hago cosas tipo start()
		node->start();
	}
}


void Simulation::doNodePolls() {
	for (GenericNode* node : Nodes) {
		// Itero por todos mis nodos y hago un poll
		node->getNodeIoContext().poll();
	}
}

#include "MyServer.h"
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <functional>
using namespace std::placeholders;
/*
Funcion para imprimir un vector de chars
void print(std::vector<unsigned char> const& input) {
	for (int i = 0; i < input.size(); i++) {
		std::cout << input.at(i);
	}
}
*/

using boost::asio::ip::tcp;

std::string make_daytime_string();

MyServer::MyServer(boost::asio::io_context& io_context)
	:	context_(io_context),
		acceptor_(io_context, tcp::endpoint(tcp::v4(), 80)),
		socket_(io_context),
		request(REQUEST_BUFFER_LENGTH),
		permitedPaths(AMOUNT_OF_PATHS)
{
	permitedPaths.push_back("randompath/radioaficionados.html");
	permitedPaths.push_back("index.html");
	permitedPaths.push_back("randompath/elviajesito.html");
}

MyServer::~MyServer()
{
}

void MyServer::start()
{
	if (socket_.is_open())
	{
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		socket_.close();
	}
	wait_connection();
}

void MyServer::wait_connection()
{
	std::cout << "wait_connection()" << std::endl;
	if (socket_.is_open())
	{
		std::cout << "Error: Can't accept new connection from an open socket" << std::endl;
		return;
	}
	acceptor_.async_accept(
		socket_,
		[this] (const boost::system::error_code& error) {
			MyServer::connection_received_cb(error);
		});
}

void MyServer::connection_received_cb(const boost::system::error_code& error)
{
	std::cout << "connection_received_cb()" << std::endl;
	if (!error) {
		read();
	}
	else {
		std::cout << error.message() << std::endl;
	}
}

void MyServer::answer()
{
	std::cout << "answer()" << std::endl;
	if (request.size() != 0) {
		msg = make_package();
		boost::asio::async_write(
			socket_,
			boost::asio::buffer(msg),
			[this](const boost::system::error_code& error, size_t bytes_sent) {
				this->response_sent_cb(error, bytes_sent);
			}
		);
	}
}

void MyServer::response_sent_cb(const boost::system::error_code& error,
								size_t bytes_sent)
{
	std::cout << "response_sent_cb()" << std::endl;
	if (!error) {
		std::cout << "Response sent. " << bytes_sent << " bytes." << std::endl;
	}
	socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	socket_.close();
	wait_connection();

}


void MyServer::read() {
	std::cout << "read()" << std::endl;
	socket_.async_receive(
		boost::asio::buffer(request.data(), request.size()),
		std::bind(&MyServer::message_received_cb, this, _1, _2)
		);
}

bool MyServer::parse_request() {
	std::cout << "parse_request()" << std::endl;
	bool ret = false;

	std::string mystring(request.begin(), request.end());

	// check if it is a get http(v1.1) request and that we are the right target
	if (mystring.size() != 0
		&& mystring.find("GET")==0
		&& mystring.find("Host: 127.0.0.1") != mystring.npos
		&& mystring.find("HTTP/1.1") != mystring.npos)
		{
		std::size_t reference_size = mystring.find("HTTP/1.1")-5;
		html_requested = mystring.substr(5, reference_size-1);
		std::cout << "Parsed without errors. The requested html is " << html_requested << std::endl;
		ret = true;
	}
	else {
		std::cout << "Request error " << std::endl;
		html_requested = "error";
	}
	return ret;
}

void MyServer::message_received_cb(const boost::system::error_code& error, size_t bytes_sent) {

	std::cout << "message_received_cb()" << std::endl;
	if (!error) {
		std::cout << "Response correctly received. " << bytes_sent << " bytes." << std::endl;
	}
	else {
		std::cout << bytes_sent << " bytes." << std::endl;
	}

	//print(request); to use this print function uncomment top of file print(...)
	parse_request();
	answer();
}


std::string MyServer::make_package()
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
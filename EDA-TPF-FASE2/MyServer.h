#pragma once
#include <boost\asio.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <vector>

#define REQUEST_BUFFER_LENGTH 100000
#define AMOUNT_OF_PATHS 3

class MyServer
{
	public:
		MyServer(boost::asio::io_context& context);
		~MyServer();

		std::vector<std::string> permitedPaths;

		void start();

	private:
		void wait_connection();
		void answer();
		void connection_received_cb(const boost::system::error_code& error);
		void response_sent_cb(const boost::system::error_code& error, size_t bytes_sent);
		void read();
		void message_received_cb(const boost::system::error_code& error, size_t bytes_sent);
		bool parse_request();
		std::string buffer_to_string(const boost::asio::streambuf& buffer);
		std::string make_package();

		std::string msg;
		std::string incoming_request;
		std::string html_requested;
		std::vector<unsigned char> request;

		boost::asio::io_context& context_;
		boost::asio::ip::tcp::socket socket_;
		boost::asio::ip::tcp::acceptor acceptor_;
};

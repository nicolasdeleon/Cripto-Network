#pragma once
#include "json.hpp"
#include <string>
#include <curl/curl.h>
#include <iostream>

#define LOCALHOST "127.0.0.1:"

using json = nlohmann::json;
using namespace std;

typedef enum { GET, POST } metodos;

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData);

class MyClient
{
public:
	MyClient(string _ip, int _port);
	MyClient();
	void configurateGETClient(int out_port);
	void configuratePOSTClient(int out_port, json& to_send);
	void sendFilterMssg(string _path, string out_ip, int out_port);
	void methodGet(string _path, string out_ip, int out_port, string block_id, int count);
	void methodPost(string _path, string out_ip, int out_port, json& to_send);
	bool performRequest(void);
	bool waiting4response();
	json getAnswer();
	void clearAnswer();
	~MyClient();

private:
	string auxiliar;
	json janswer;
	CURLMcode errorMulti;
	CURL* handler;
	CURL* multiHandler;
	string ip, answer, url, host;
	string Erorr_string;
	int port;
	int recibiendoInfo;
	metodos metodo;
};


#include "MyClient.h"
#include "json.hpp"

/*ACLARACIONES VARIAS

** Parte del codigo esta muy repetido, despues ver de comprimir lo que se pueda en las mismas funciones


*/
MyClient::MyClient(std::string _ip, int _port) //crearlo con el local host
{
	ip = _ip; //igualarlo a local post sino (hay un define) LOCALHOST
	port = _port;
	handler = curl_easy_init();
	if (!handler)
	{
		Erorr_string = "No se pudo inicializar curl easy";
	}
	multiHandler = curl_multi_init();
	if (!multiHandler)
	{
		Erorr_string = "No se pudo inicializar curl multi";
	}
	recibiendoInfo = 0;
}

MyClient::MyClient()
{
	recibiendoInfo = 0;
}

MyClient::~MyClient()
{
	curl_multi_remove_handle(multiHandler, handler);
	curl_easy_cleanup(handler);
	curl_multi_cleanup(multiHandler);
}

void MyClient::methodGet(string _path, string out_ip, int out_port, string block_id, int count) //path vendria a ser lo que queres obtener
{
	metodo = GET; //de momento no lo uso, ver si es relevante
	host = out_ip + ":" + to_string(out_port);
	url = "http://" + host + "/eda_coin/" + _path + "?block_id=" + block_id + "&count=" +  to_string(count); //con la url le termino pasando que quiero que me devuelva
	configurateGETClient(out_port);
	recibiendoInfo = 1;
}

void MyClient::methodPost(string _path, string out_ip, int out_port, json to_send) //path vendria a ser lo que queres enviar
{
	metodo = POST; //de momento no lo uso, ver si es relevante
	host = out_ip + ":" + to_string(out_port);
	url = "http://" + host + "/eda_coin/" + _path; //con la url le termino pasando que quiero que me devuelva
	configuratePOSTClient(out_port, to_send);
	recibiendoInfo = 1;
}


//Configurates client.
void MyClient::configurateGETClient(int out_port) {
	answer.clear();

	//setea handler y multihandler
	curl_multi_add_handle(multiHandler, handler);

	//setea la url para leer de ahi
	curl_easy_setopt(handler, CURLOPT_URL, url.c_str());

	//Setea protocolo (HTTP).
	curl_easy_setopt(handler, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);

	/*Setea puerto que recibe la request*/
	curl_easy_setopt(handler, CURLOPT_PORT, out_port);

	/*setea el puerto que envia la request.*/
	curl_easy_setopt(handler, CURLOPT_LOCALPORT, port);

	//setea el callback y guarda la respuesta en answer
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback);

	curl_easy_setopt(handler, CURLOPT_WRITEDATA, &answer);
}

//Configurates client.
void MyClient::configuratePOSTClient(int out_port, json to_send) {
	answer.clear();

	struct curl_slist* list = nullptr;
	
	
	//Esto no se si va igualmente

	//Le decimos a CURL que vamos a mandar URLs codificadas y adem�s en formato UTF8.
	list = curl_slist_append(list, "Content-Type: application/json; Charset: UTF-8");
	curl_easy_setopt(handler, CURLOPT_HTTPHEADER, list);



	//Le decimos a CURL que trabaje con credentials.
	string to_send_test = to_send.dump().c_str();
	curl_easy_setopt(handler, CURLOPT_POSTFIELDSIZE, to_send.dump().size());
	curl_easy_setopt(handler, CURLOPT_POSTFIELDS, to_send.dump().c_str());

	//setea handler y multihandler
	curl_multi_add_handle(multiHandler, handler);

	//setea la url para leer de ahi
	curl_easy_setopt(handler, CURLOPT_URL, url.c_str());

	// Si la p�gina nos redirije a alg�n lado, le decimos a curl que siga dicha redirecci�n.
	curl_easy_setopt(handler, CURLOPT_FOLLOWLOCATION, 1L);

	//Setea protocolo (HTTP).
	curl_easy_setopt(handler, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
		
	/*Setea puerto que recibe la request*/
	curl_easy_setopt(handler, CURLOPT_PORT, out_port);

	/*setea el puerto que envia la request.*/
	curl_easy_setopt(handler, CURLOPT_LOCALPORT, port);

	//setea el callback y guarda la respuesta en answer
	curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, &writeCallback);

	curl_easy_setopt(handler, CURLOPT_WRITEDATA, &answer);
}

bool MyClient::performRequest(void)
{
	if (recibiendoInfo) {
		errorMulti = curl_multi_perform(multiHandler, &recibiendoInfo);
		if (errorMulti != CURLE_OK) {
			curl_easy_cleanup(handler);
			curl_multi_cleanup(multiHandler);
		}

	}
	else if (answer != "") {
			curl_easy_cleanup(handler);
			curl_multi_cleanup(multiHandler);
			janswer = json::parse(answer);
			cout << "janswer == " << janswer << endl;
			answer = "";		
	}

	return true;
}



//Callback with string as userData.
size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userData) {
	std::string* userDataPtr = (std::string*) userData;

	userDataPtr->append(ptr, size * nmemb);

	return size * nmemb;
}

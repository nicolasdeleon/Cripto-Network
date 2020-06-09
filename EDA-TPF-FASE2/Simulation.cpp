#include "Simulation.h"

Simulation::Simulation()
{

}




Simulation::~Simulation() {
	for (GenericNode* node : Nodes) {
		if(node)
			delete(node);
	}
	for (boost::asio::io_context* context : contexts) {
		if(context)
			delete(context);
	}
}


GenericNode* Simulation::addNode(string ip, unsigned int port, NodeType NodeType) {
	bool res = true;
	if (port % 2) {
		cout << "ESTO NO DEBERIA PASAR!! PUERTO IMPAR!!" << endl;
	}

	boost::asio::io_context* context = new boost::asio::io_context();
	contexts.push_back(context);
	GenericNode* newNode = nullptr;
	if (NodeType == NodeType::SPV) {
		newNode = new SPVNode(*context, ip, port);
	}
	else if (NodeType == NodeType::FULL) {
		newNode = new FullNode(*context, ip, port);
	}
	if (newNode == nullptr) {
		cout << "Problema inicializando el nodo" << endl;
		res = false;
	}

	Nodes.push_back(newNode);
	return newNode;
}

void Simulation::addNodeAndStart(string ip, unsigned int port, NodeType NodeType) {
	bool res = addNode(ip, port, NodeType);
	if (res)
		Nodes.back()->start();
}

bool Simulation::deleteNode(string ip, unsigned int port) {
	string address = createAddress(ip, port);
	bool res = false;
	for (int i = 0; i < Nodes.size(); i++) {
		if (Nodes[i]->getAddress() == address) {
			delete(Nodes[i]);
			Nodes.erase(Nodes.begin() + i);
			delete(contexts[i]);
			contexts.erase(contexts.begin() + i);
			res = true;
		}
	}
	if (!res)
		cout << "Can't delete un-existent node" << endl;

	return res;
}


bool Simulation::deleteConnection(string origin_address, string destiny_address) {

	bool res = false;
	for (GenericNode* node : Nodes) {
		if (node->getAddress() == origin_address) {
			node->deleteConnection(destiny_address);
			res = true;
		}
	}

	if (!res)
		cout << "Nodo origen no encontrado. �Te olvidaste de cargarlo?" << endl;

	return res;
}

bool Simulation::createConnection(string ip_origen, int puerto_origen, string ip_destino, int puerto_destino) {
	string origin_address = createAddress(ip_origen, puerto_origen);
	string destiny_address = createAddress(ip_destino, puerto_destino);
	bool res = false;
	for (GenericNode* node : Nodes) {
		if (node->getAddress() == origin_address) {
			node->addConnection(destiny_address);
			res = true;
		}
	}

	if (!res)
		cout << "Nodo origen no encontrado. �Te olvidaste de cargarlo?" << endl;

	return res;

} 

string Simulation::createAddress(string ip, int port) {
	string address = ip + ":" + to_string(port);
	return address;
}

void Simulation::giveAddress2Nodes(vector<string>& addss)
{
	for (auto node : Nodes) {
		node->giveAvailableNodes(addss);
	}
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
		node->doPolls();
	}
}

vector<GenericNode*> Simulation::getNodes() {
	return Nodes;
}

void Simulation::sendMessageFromNode2Node(
	string ip_origen,
	unsigned int port_origen,
	string ip_destino,
	unsigned int port_destino,
	MessageIds request_id, json& Json,
	unsigned int block_id, unsigned int cant) {

	string origin_address = createAddress(ip_origen, port_origen);
	bool res = false;
	for (GenericNode* node : Nodes) {
		if (node->getAddress() == origin_address) {
			node->send_request(request_id, ip_destino, port_destino, Json, block_id, cant);
		}
	}
}

void Simulation::sendTransaction(string origin_adress, string target_adress, int amount) {
	
	int port = stoi(target_adress.substr(target_adress.find(":") + 1));
	string ip = target_adress.substr(0, target_adress.find(":"));
	
	cout << target_adress << " " << amount;

	for (GenericNode* node : Nodes) {
		if (node->getAddress() == origin_adress) {
			static_cast<FullNode*>(node)->sendTX("send_tx", ip, port, { amount }, { "32423" });
			
		}
	}
}

string Simulation::getRequestAnswer(string address) {
	string answer = "Node not found";
	for (GenericNode* node : Nodes) {
		if (node->getAddress() == address) {
			answer = node->getClientRequestAnswer();
		}
	}
	return answer;
}

string get_address_ip(string& address) {
	return address.substr(0, address.find(":"));
}

unsigned int get_address_port(string& addres) {
	return 	stoi(addres.substr(addres.find(":") + 1));
}

bool Simulation::appendNode(string& my_ip, unsigned int& my_port, NodeType my_type, vector<string>& neighborhood) {
	bool res = true;

	if ((my_type == NodeType::FULL && neighborhood.size() < 1) ||
		(my_type == NodeType::SPV && neighborhood.size() < 2)) {
		res = false;
		cout << "Cantidad de conexiones incorrectas para el tipo de nodo" << endl;
	}

	GenericNode* my_node = addNode(my_ip, my_port, my_type);
	my_node->start();
	
	for (string neighbor : neighborhood) {
		string neighbor_ip = get_address_ip(neighbor);
		unsigned int neighbor_port = get_address_port(neighbor);
		if (!createConnection(my_node->getIP(), my_node->getPort(), neighbor_ip, neighbor_port))
			res = false;
	}

	if (!res) {
		// si hasta aca hubo algun problema retorno para no pinguear a nadie
		deleteNode(my_ip, my_port);
		cout << "No se pudo appendear el nodo" << endl;
		return res;
	}
	
	my_node->startAppend();
	
	return res;
}


bool Simulation::areFullReady() {
	bool ready = true;
	for (auto node : Nodes) {
		if (node->getType() == NodeType::FULL && !(node->getState())) {
			ready = false;
		}
	}
	return ready;
}

void Simulation::connectSPV()
{
		
		GenericNode* node1, *node2;
		do {
			int randNum = rand() % Nodes.size();
			node1 = Nodes[randNum];
		} while (node1->getType() != NodeType::FULL);
		do {
			int randNum = rand() % Nodes.size();
			node2 = Nodes[randNum];
		} while (node2->getType() != NodeType::FULL || node1 == node2);

		string MyIp = "127.0.0.1";
		unsigned int myPort = static_cast<unsigned int>(spvGenNodes.back());
		spvGenNodes.pop_back();

		vector<string> neighborhood = { (node1->getIP() + ":" + to_string(node1->getPort())),
										(node2->getIP() + ":" + to_string(node2->getPort())) };

		appendNode(MyIp, myPort, NodeType::SPV, neighborhood);
}

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#include "blockchainHandler.h"

#include <fstream>
#include <sstream>
#include <iomanip>


size_t blockchainHandler::chainSize() { return BlockChainJSON.size(); }

blockchainHandler::blockchainHandler() : keys{ "blockid", "height", "merkleroot", "nTx", "nonce", "previousblockid", "tx" } {
};

string blockchainHandler::hexCodexASCII(unsigned int number) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << number;

	return ss.str();

}

bool blockchainHandler::parseallOk(string str, string * errorString)
{

	bool retVal = false;
	//Abro el archivo y lo asigno a mi variable miembro blocks (de tipo json)
	ifstream blocks_file(str, ifstream::binary);

	try {
		blocks_file >> BlockChainJSON;
		if (!BlockChainJSON.empty()) { //chequea que no esté vacío así no crashea todo con el parser

			retVal = true;
			//Recorro todos los elementos de la vectora y me fijo si cada diccionario tiene 7 keys
			BlockChainJSON.size();
			for (int i = 0; i < BlockChainJSON.size(); i++)
			{
				if (BlockChainJSON[i].size() != 7)
				{
					//cout << "Error uno de los bloques tiene menos de 7 keys" << endl;
					*errorString = "Error parsing \nwrong key number!";
					retVal = false;
				}
			}

			//Recorro nuevamente todos los elementos de la vectora y todos los elementos del diccionario 
			//y para cada uno de ellos me fijo que su nombre corresponda con los del vector keys
			for (auto item : BlockChainJSON)
			{
				for (int i = 0; i < item.size(); i++) //item.size es 7 igual ya lo comprobamos arriba
				{
					if (item.find(keys[i]) == item.end())
					{
						//cout << "Error, una de las keys no corresponde con lo esperado" << endl;
						*errorString = "Error parsing \nwrong key value!";
						retVal = false;
					}
				}
			}

		}
	}
	catch (json::parse_error& e)
	{
		std::cerr << e.what() << std::endl;
		*errorString = "Error parsing \nEmpty or corrupt file!";
	}
	return retVal;
}

vector<vector<string>> blockchainHandler::makeMerkleTree(int blockNumber) {
	
	vector<vector<string>> levels;
	
	// Hojas
	vector<string> currentLevel;

	vector<string> prehashIDs;
	for (auto transaction : BlockChainJSON[blockNumber]["tx"]) {
		
		string id;
		for (auto tx : transaction["vin"]) {
			id += string(tx["txid"].get<string>());
		}
		prehashIDs.push_back(id);
	}
	if (prehashIDs.size() % 2) {
		prehashIDs.push_back(prehashIDs.back());
	}
	

	// hashes

	currentLevel.clear();
	for (string oldId: prehashIDs) {

		unsigned char* oldID_char = new unsigned char[oldId.length() + 1]; //Copiado de stack overflow 
		strcpy((char*)oldID_char, oldId.c_str());

		unsigned int newID = generateID(oldID_char);
		currentLevel.push_back(hexCodexASCII(newID));
	}
	// Borrar lo de abajo
	if (currentLevel.size() % 2) {
		currentLevel.push_back(levels[1].back());
	}
	levels.push_back(currentLevel);


	while (levels.back().size() > 1) {
		
		currentLevel.clear();

		if (!(levels.back().size() % 2)) {
			// Uno los hashes anteriores en el actual
			for (int i = 1; i < levels.back().size(); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
		}
		else {
			int i;
			for (i = 1; i < (levels.back().size() - 1); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
			currentLevel.push_back(levels.back()[i-1] + levels.back()[i-1]);
		}

		//re-hasheo el nivel actual

		for (int i = 0; i < currentLevel.size(); i ++) {
			
			unsigned char* oldID = new unsigned char[currentLevel[i].length() + 1]; //Copiado de stack overflow 
			strcpy((char*)oldID, currentLevel[i].c_str());

			unsigned int newID = generateID(oldID);
			currentLevel[i] = hexCodexASCII(newID);
		}

		//Agrego el nivel actual al vector
		levels.push_back(currentLevel);
	}
	merkleroot = BlockChainJSON[blockNumber]["merkleroot"].get<string>();
	//cout << "Nuestra merkle root: " << levels.back().back() << endl; 
	//cout << "La merkle root del bloque: " << BlockChainJSON[blockNumber]["merkleroot"] << endl; 


	return levels;
}

vector<vector<string>> blockchainHandler::makeMerkleTree(json& BlockJSON) {

	cout << BlockJSON.dump(2) << endl;
	vector<vector<string>> levels;

	// Hojas
	vector<string> currentLevel;

	vector<string> prehashIDs;
	for (auto transaction : BlockJSON["tx"]) {

		string id;
		for (auto tx : transaction["vin"]) {
			id += string(tx["txid"].get<string>());
		}
		prehashIDs.push_back(id);
	}
	if (prehashIDs.size() % 2) {
		prehashIDs.push_back(prehashIDs.back());
	}


	// hashes

	currentLevel.clear();
	for (string oldId : prehashIDs) {

		unsigned char* oldID_char = new unsigned char[oldId.length() + 1]; //Copiado de stack overflow 
		strcpy((char*)oldID_char, oldId.c_str());

		unsigned int newID = generateID(oldID_char);
		currentLevel.push_back(hexCodexASCII(newID));
	}
	// Borrar lo de abajo
	if (currentLevel.size() % 2) {
		currentLevel.push_back(levels[1].back());
	}
	levels.push_back(currentLevel);


	while (levels.back().size() > 1) {

		currentLevel.clear();

		if (!(levels.back().size() % 2)) {
			// Uno los hashes anteriores en el actual
			for (int i = 1; i < levels.back().size(); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
		}
		else {
			int i;
			for (i = 1; i < (levels.back().size() - 1); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
			currentLevel.push_back(levels.back()[i - 1] + levels.back()[i - 1]);
		}

		//re-hasheo el nivel actual

		for (int i = 0; i < currentLevel.size(); i++) {

			unsigned char* oldID = new unsigned char[currentLevel[i].length() + 1]; //Copiado de stack overflow 
			strcpy((char*)oldID, currentLevel[i].c_str());

			unsigned int newID = generateID(oldID);
			currentLevel[i] = hexCodexASCII(newID);
		}

		//Agrego el nivel actual al vector
		levels.push_back(currentLevel);
	}
	//merkleroot = BlockChainJSON[blockNumber]["merkleroot"].get<string>();
	//cout << "Nuestra merkle root: " << levels.back().back() << endl; 
	//cout << "La merkle root del bloque: " << BlockChainJSON[blockNumber]["merkleroot"] << endl; 


	return levels;
}
/*
vector<vector<string>> blockchainHandler::makeMerkleTree(json& block) {

	vector<vector<string>> levels;

	// Hojas
	vector<string> currentLevel;

	vector<string> prehashIDs;
	for (auto transaction : BlockChainJSON[blockNumber]["tx"]) {

		string id;
		for (auto tx : transaction["vin"]) {
			id += string(tx["txid"].get<string>());
		}
		prehashIDs.push_back(id);
	}
	if (prehashIDs.size() % 2) {
		prehashIDs.push_back(prehashIDs.back());
	}


	// hashes

	currentLevel.clear();
	for (string oldId : prehashIDs) {

		unsigned char* oldID_char = new unsigned char[oldId.length() + 1]; //Copiado de stack overflow 
		strcpy((char*)oldID_char, oldId.c_str());

		unsigned int newID = generateID(oldID_char);
		currentLevel.push_back(hexCodexASCII(newID));
	}
	// Borrar lo de abajo
	if (currentLevel.size() % 2) {
		currentLevel.push_back(levels[1].back());
	}
	levels.push_back(currentLevel);


	while (levels.back().size() > 1) {

		currentLevel.clear();

		if (!(levels.back().size() % 2)) {
			// Uno los hashes anteriores en el actual
			for (int i = 1; i < levels.back().size(); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
		}
		else {
			int i;
			for (i = 1; i < (levels.back().size() - 1); i += 2) {
				currentLevel.push_back(levels.back()[i - 1] + levels.back()[i]);
			}
			currentLevel.push_back(levels.back()[i - 1] + levels.back()[i - 1]);
		}

		//re-hasheo el nivel actual

		for (int i = 0; i < currentLevel.size(); i++) {

			unsigned char* oldID = new unsigned char[currentLevel[i].length() + 1]; //Copiado de stack overflow 
			strcpy((char*)oldID, currentLevel[i].c_str());

			unsigned int newID = generateID(oldID);
			currentLevel[i] = hexCodexASCII(newID);
		}

		//Agrego el nivel actual al vector
		levels.push_back(currentLevel);
	}
	merkleroot = BlockChainJSON[blockNumber]["merkleroot"].get<string>();
	//cout << "Nuestra merkle root: " << levels.back().back() << endl; 
	//cout << "La merkle root del bloque: " << BlockChainJSON[blockNumber]["merkleroot"] << endl; 


	return levels;
}*/ 

unsigned int blockchainHandler::generateID(unsigned char* str)
{
	unsigned int id = 0;
	int c;
	while (c = *str++)
		id = c + (id << 6) + (id << 16) - id;
	return id;
}

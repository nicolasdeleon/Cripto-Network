#pragma once

#include "json.hpp"
#include <string>
#include <vector>
#include <iterator>
#include <list> 
#include <iostream>
#include <map>

using namespace std;
using json = nlohmann::json;

class blockchainHandler {
public:
	blockchainHandler();
	bool parseallOk(string filepath, string * ErrorString = 0);
	size_t chainSize();
	string getMerkleroot() { return merkleroot; }
	vector<string> getKeys() { return keys; }
	string hexCodexASCII(unsigned int);
	vector<vector<string>> makeMerkleTree(int blockNumber);
	vector<vector<string>> makeMerkleTree(json& block);
	json BlockChainJSON;
	
private:
	vector<string> keys;
	string merkleroot;
	unsigned int generateID(unsigned char* str);
	
};

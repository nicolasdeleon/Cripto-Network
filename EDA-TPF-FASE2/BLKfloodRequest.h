#pragma once


#pragma once

#include <string>
#include <queue>
#include <vector>

using namespace std;

class BLKfloodRequest
{
public:

    BLKfloodRequest(json& blockJson, vector<string>& neighbors) : blockJson(blockJson) {
        for (auto x : neighbors) {
            keys_left.push(x);
        }
    }

    string get_next_neighbor() {
        string temp = keys_left.front();
        keys_left.pop();
        return temp;
    }

    bool empty() {
        return !(keys_left.size());
    }

    json get_block() {
        return blockJson;
    }

private:

    json blockJson;
    queue<string> keys_left;

};

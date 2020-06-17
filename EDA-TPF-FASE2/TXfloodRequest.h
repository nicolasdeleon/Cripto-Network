#pragma once

#include <string>
#include <queue>
#include <vector>

using namespace std;

class TXfloodRequest
{
public:

    TXfloodRequest(vector<int>& amt, vector<string>& pubIDs, vector<string>& neighbors) : ids(pubIDs), amounts(amt) { 
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

    vector<string> get_ids() {
        return ids;
    }
    vector<int> get_amounts() {
        return amounts;
    }

private:

    vector<int> amounts;
    vector<string> ids;
    queue<string> keys_left;

};


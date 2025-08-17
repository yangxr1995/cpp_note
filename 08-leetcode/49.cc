#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Solution {
public:
    vector<vector<string>> groupAnagrams(vector<string>& strs) {
        unordered_map<string, vector<string>> tb;
        for (const auto &str : strs) {
            string str_sort = str;
            sort(str_sort.begin(), str_sort.end());
            tb[str_sort].emplace_back(str);
        }
        vector<vector<string>> ret;
        for (const auto &pair : tb) {
            ret.emplace_back(pair.second);
        }
        return ret;
    }
};

int main (int argc, char *argv[]) {
    Solution s;
    vector<string> strs = {
            "eat","tea","tan","ate","nat","bat"
        // "ten", "net", "nat", "atn", "eth", "hta",
    };
    auto str2 = s.groupAnagrams(strs);
    for (auto &item  : str2) {
        cout << "[ ";
        for (auto &str  : item) {
            cout << str << " ";
        }
        cout << "] ";
    }
        cout << endl;
    
    return 0;
}

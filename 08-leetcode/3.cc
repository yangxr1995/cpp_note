#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    int lengthOfLongestSubstring(string s) {
        
        int i, j;
        unordered_map<char, int> tb;
        int maxlen = 0;
        for (i = 0, j = 0; j < s.length(); ++j) {
            auto tmp = tb.find(s[j]);

            if (tmp != tb.end() && tmp->second >= i) {
                i = tmp->second + 1;
            }

            tb[s[j]] = j;
            maxlen = max(maxlen, j - i + 1);
        }

        return maxlen;
    }
};

int main (int argc, char *argv[]) {
    // string s = "abcabcbb";
    // string s = "pwwkew";
    string s = "abba";
    Solution sol;
    cout << sol.lengthOfLongestSubstring(s) << endl;
    
    return 0;
}

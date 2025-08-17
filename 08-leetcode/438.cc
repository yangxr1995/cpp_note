#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace std;

class Solution {
public:
    vector<int> findAnagrams(string s, string p) {
        vector<int> sinfo(26, 0), pinfo(26, 0);
        vector<int> ret;

        if (s.size() < p.size())
            return ret;

        for (int i = 0; i < p.size(); ++i) {
            pinfo[p[i] - 'a']++;
            sinfo[s[i] - 'a']++;
        }

        if (pinfo == sinfo) {
            ret.emplace_back(0);
        }

        int len = p.size();
        for (int i = 1; i <= s.size() - len; ++i) {
            sinfo[s[i - 1] - 'a']--;
            sinfo[s[i + len - 1] - 'a']++;
            if (pinfo == sinfo)
                ret.emplace_back(i);
        }

        return ret;
    }
};

int main (int argc, char *argv[]) {
    Solution s;
    // string s1("cbaebabacd");
    // string s2("abc");


    string s1("a");
    string s2("abc");
    // string s1("abab");
    // string s2("ab");
    vector<int> ret = s.findAnagrams(s1, s2);
    copy(ret.begin(), ret.end(), ostream_iterator<int>(cout, ", "));
    
    return 0;
}

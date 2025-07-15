// 编写一个函数来查找字符串数组中的最长公共前缀。
//
// 如果不存在公共前缀，返回空字符串 ""。

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Solution {
    public:
        string longestCommonPrefix(vector<string>& strs) {

            if (strs.empty())
                return "";

            if (strs.size() == 1)
                return strs[0];

            string &first_str = strs[0];
            for (int i = 0; i < first_str.length(); ++i) {
                for (auto pos = strs.begin() + 1; pos != strs.end(); ++pos) {
                    if (first_str[i] != (*pos)[i]) {
                        return first_str.substr(0, i);
                    }
                }
            }

            return first_str;
        }
};

int main (int argc, char *argv[]) {

    Solution s;
    vector<string> strs1 = {
        "flower",
        "flow",
        "flight"
    };
    cout << "Test Case 1: " << s.longestCommonPrefix(strs1) << endl;  // 输出 "fl"

    vector<string> strs2 = {
        "dog",
        "racecar",
        "car"
    };
    cout << "Test Case 2: " << s.longestCommonPrefix(strs2) << endl;  // 输出 ""

    vector<string> strs3 = {
        "apple"
    };
    cout << "Test Case 3: " << s.longestCommonPrefix(strs3) << endl;  // 输出 "apple"

    return 0;
}

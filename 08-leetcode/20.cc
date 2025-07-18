// 给定一个只包括 '('，')'，'{'，'}'，'['，']' 的字符串 s ，判断字符串是否有效。
//
// 有效字符串需满足：
//
// 左括号必须用相同类型的右括号闭合。
// 左括号必须以正确的顺序闭合。
// 每个右括号都有一个对应的相同类型的左括号。

#include <ios>
#include <iostream>
#include <string>
#include <stack>
using namespace std;

class Solution {
public:
    bool isValid(string s) {
        stack<char> st;

        if (s.empty())
            return true;

        for (char c  : s) {
            if (c == '(' || c == '{' || c == '[') {
                st.push(c);
            }
            else {
                if (st.empty())
                    return false;

                char c2 = st.top();
                st.pop();

                if (
                  !((c == ')' && c2 == '(') ||
                    (c == '}' && c2 == '{') ||
                    (c == ']' && c2 == '[')))
                    return false;
            }
        }

        if (st.empty())
            return true;
        return false;
    }
};

int main (int argc, char *argv[]) {

    Solution s;
    cout << boolalpha;
    cout << s.isValid("[()]") << endl;
    cout << s.isValid("[()]{}") << endl;
    cout << s.isValid("[()]{}[]") << endl;
    cout << s.isValid("[()]}[]") << endl;
    
    return 0;
}

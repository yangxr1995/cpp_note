#include <iostream>
using namespace std;

// 给你一个整数 x ，如果 x 是一个回文整数，返回 true ；否则，返回 false 。
//
// 回文数是指正序（从左向右）和倒序（从右向左）读都是一样的整数。

class Solution {
public:
    bool isPalindrome(int x) {
        if (x < 0)
            return false;
        int bak = x;
        long long reverse = 0;
        while (x) {
            reverse = reverse * 10 + x%10;
            x /= 10;
        }
        return bak == reverse;
    }
};

int main (int argc, char *argv[]) {

    Solution s;
    cout << s.isPalindrome(10) << endl;
    cout << s.isPalindrome(11) << endl;
    cout << s.isPalindrome(122) << endl;
    cout << s.isPalindrome(121) << endl;
    
    return 0;
}

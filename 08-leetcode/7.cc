#include <iostream>
#include <limits.h>
using namespace std;

// 给你一个 32 位的有符号整数 x ，返回将 x 中的数字部分反转后的结果。
// 如果反转后整数超过 32 位的有符号整数的范围 [−231,  231 − 1] ，就返回 0。
// 假设环境不允许存储 64 位整数（有符号或无符号）。

class Solution {
public:
    int reverse(int x) {
        int ret = 0;
        int c;
        while (x) {
            c = x % 10;
            x = x / 10;
            if (ret > INT_MAX/10 || (ret == INT_MAX/10 && c > 7)) return 0;
            if (ret < INT_MIN/10 || (ret == INT_MIN/10 && c < -8)) return 0;
            ret = ret * 10 + c;
        }
        return ret;
    }
};

int main (int argc, char *argv[]) {
    Solution s;
    cout << s.reverse(123) << endl;
    cout << s.reverse(-123) << endl;
    cout << s.reverse(-1201) << endl;
    
    return 0;
}

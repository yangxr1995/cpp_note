// 给你两个二进制字符串 a 和 b ，以二进制字符串的形式返回它们的和。
//
// 示例 1：
//
// 输入:a = "11", b = "1"
// 输出："100"
// 示例 2：
//
// 输入：a = "1010", b = "1011"
// 输出："10101"

#include <string>
#include <algorithm>
#include <iostream>
#include <bitset>
using namespace std;

class Solution {
public:
    string addBinary(string a, string b) {

        string ret;
        int i, j, sum;

        i = a.size() - 1;
        j = b.size() - 1;
        sum = 0;
        while (i >= 0 || j >= 0 || sum > 0) {

            if (i >= 0)
                sum += a[i--] - '0';
            if (j >= 0)
                sum += b[j--] - '0';

            cout << "sum : " << sum << endl;

            ret.push_back((sum % 2) ? '1' : '0');
            sum = sum >> 1;
            cout << "sum : " << sum << endl;
        }
        reverse(ret.begin(), ret.end());
        return ret;
    }
};

int main (int argc, char *argv[]) {
    
    Solution s;

    // 110
    cout << s.addBinary("11", "1") << endl;
    cout << s.addBinary("0", "0") << endl;
//     string a =
// "10100000100100110110010000010101111011011001101110111111111101000000101111001110001111100001101";
// string b =
// "110101001011101110001111100110001010100001101011101010000011011011001011101111001100000011011110011";
//     cout << s.addBinary(a, b) << endl;

    return 0;
}

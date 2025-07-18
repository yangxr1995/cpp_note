// 给你两个字符串 haystack 和 needle ，请你在 haystack 字符串中找出 needle 字符串的第一个匹配项的下标（下标从 0 开始）。如果 needle 不是 haystack 的一部分，则返回  -1 。
//
// 示例 1：
//
// 输入：haystack = "sadbutsad", needle = "sad"
// 输出：0
// 解释："sad" 在下标 0 和 6 处匹配。
// 第一个匹配项的下标是 0 ，所以返回 0 。
// 示例 2：
//
// 输入：haystack = "leetcode", needle = "leeto"
// 输出：-1
// 解释："leeto" 没有在 "leetcode" 中出现，所以返回 -1 。
 
#include <string>
#include <iostream>
using namespace std;

class Solution {
public:
    int strStr(string haystack, string needle) {
        if (needle.size() == 0)
            return 0;
        if (needle.size() > haystack.size())
            return -1;

        int i, j;
        for (i = 0; i < haystack.size(); ++i) {
            for (j = 0; j < needle.size(); ++j) {
                if (haystack[i + j] != needle[j])
                    break;
            }
            if (j == needle.size())
                return i;
        }
        return -1;
    }
};

int main (int argc, char *argv[]) {
    
    return 0;
}

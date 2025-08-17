// 给定一个未排序的整数数组 nums ，找出数字连续的最长序列（不要求序列元素在原数组中连续）的长度。
//
// 请你设计并实现时间复杂度为 O(n) 的算法解决此问题。
//
//
//
// 示例 1：
//
// 输入：nums = [100,4,200,1,3,2]
// 输出：4
// 解释：最长数字连续序列是 [1, 2, 3, 4]。它的长度为 4。
// 示例 2：
//
// 输入：nums = [0,3,7,2,5,8,4,6,0,1]
// 输出：9
// 示例 3：
//
// 输入：nums = [1,0,1,2]
// 输出：3

#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <vector>
#include <iostream>
using namespace std;


class Solution {
public:
    int longestConsecutive(vector<int>& nums) {
        unordered_set<int> set_num;
        for (int n  : nums) {
            set_num.insert(n);
        }
        int cur, max;
        max = 0;
        for (int n  : set_num) {
            if (set_num.find(n - 1) == set_num.end()) {
                cur = 1;

                while (set_num.find(n + 1) != set_num.end()) {
                    ++n;
                    ++cur;
                }

                if (cur > max)
                    max = cur;
            }
        }
        return max;
    }
};

int main (int argc, char *argv[]) {
    Solution s;
    // vector<int> v = {10, 4, 20, 1, 3, 2};
    // vector<int> v = {10};
    vector<int> v = {1,0,1,2};
    cout << s.longestConsecutive(v);
    
    return 0;
}

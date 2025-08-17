// 给你一个整数数组 nums ，判断是否存在三元组 [nums[i], nums[j], nums[k]] 满足 i != j、i != k 且 j != k ，同时还满足 nums[i] + nums[j] + nums[k] == 0 。请你返回所有和为 0 且不重复的三元组。
//
// 注意：答案中不可以包含重复的三元组。
//
//
//
//
//
// 示例 1：
//
// 输入：nums = [-1,0,1,2,-1,-4]
// 输出：[[-1,-1,2],[-1,0,1]]
// 解释：
// nums[0] + nums[1] + nums[2] = (-1) + 0 + 1 = 0 。
// nums[1] + nums[2] + nums[4] = 0 + 1 + (-1) = 0 。
// nums[0] + nums[3] + nums[4] = (-1) + 2 + (-1) = 0 。
// 不同的三元组是 [-1,0,1] 和 [-1,-1,2] 。
// 注意，输出的顺序和三元组的顺序并不重要。
// 示例 2：
//
// 输入：nums = [0,1,1]
// 输出：[]
// 解释：唯一可能的三元组和不为 0 。
// 示例 3：
//
// 输入：nums = [0,0,0]
// 输出：[[0,0,0]]
// 解释：唯一可能的三元组和为 0 。

#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>
#include <iostream>
using namespace std;

class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        
        int i, j, k, last;
        sort(nums.begin(), nums.end());
        vector<vector<int>> ret;
        for (i = 0; i < nums.size() - 2; ++i) {
            if (i > 0 && nums[i] == nums[i - 1])
                continue;
            int right = nums.size() - 1;
            int left = i + 1;
            while (left < right) {
                if (right < nums.size() -1 && nums[right] == nums[right + 1]) {
                    --right;
                    continue;
                }
                if (left > i + 1 && nums[left] == nums[left - 1]) {
                    ++left;
                    continue;
                }
                int sum = nums[i] + nums[left] + nums[right];
                if (sum == 0) {
                    ret.push_back({nums[i], nums[left], nums[right]});
                    ++left;
                    --right;
                }
                else if (sum > 0) {
                    right--;
                }
                else {
                    left++;
                }

            }
        }

        return ret;
    }
};

int main (int argc, char *argv[]) {

    Solution s;
    // -4 -1 -1 0 1 2 
    vector<int> v = {-1,0,1,2,-1,-4};

    auto vv = s.threeSum(v);
    for (const auto &item  : vv) {
        cout << "[ ";
        copy(item.begin(), item.end(), ostream_iterator<int>(cout, " "));
        cout << "], ";
    }
    cout << endl;

    return 0;
}

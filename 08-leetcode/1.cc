#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

// 给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出 和为目标值 target  的那 两个 整数，并返回它们的数组下标。
// 你可以假设每种输入只会对应一个答案，并且你不能使用两次相同的元素。
// 你可以按任意顺序返回答案。

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        unordered_map<int, int> nums_map;
        vector<int> ret;
        for (int i = 0; i < nums.size(); ++i) {
            int val1 = nums[i];
            int val2 = target - val1;
            if (nums_map.find(val2) != nums_map.end()) {
                ret.push_back(nums_map[val2]);
                ret.push_back(i);
                break;
            }
            nums_map[val1] = i;
        }
        return ret;
    }
};

int main() {
    vector<int> nums = {2, 3, 4, 5, 8};
    Solution s;
    vector<int> v = s.twoSum(nums, 7);
    for (int n  : v) {
        cout << n << " ";
    }
    cout << endl;
}

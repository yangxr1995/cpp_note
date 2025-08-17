#include <cstdint>
#include <unordered_map>
#include <vector>
#include <iostream>
using namespace std;

class Solution {
public:
    int subarraySum(vector<int>& nums, int k) {
        
        
        unordered_map<int, int> prefixSumCount;
        prefixSumCount[0] = 1;

        int sum = 0;
        int ret = 0;
        for (auto n  : nums) {
            sum += n;

            int target = sum - k;
            if (prefixSumCount.find(target) != prefixSumCount.end()) {
                ret += prefixSumCount[target];
            }

            ++prefixSumCount[sum];
        }

        return ret;
    }
};

int main (int argc, char *argv[]) {
    // vector<int> v = {1,1,1};
    // vector<int> v = {1, 2, 3};
    vector<int> v = {-1, -1, 1};
    Solution s;
    cout << s.subarraySum(v, 1) << endl;
    
    return 0;
}

// 给定一个长度为 n 的整数数组 height 。有 n 条垂线，第 i 条线的两个端点是 (i, 0) 和 (i, height[i]) 。
//
// 找出其中的两条线，使得它们与 x 轴共同构成的容器可以容纳最多的水。
//
// 返回容器可以储存的最大水量。
//
// 说明：你不能倾斜容器。


#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    int maxArea(vector<int>& height) {

        int i, j;
        int cur, max = 0;
        for (i = 0, j = height.size() - 1; i != j;) {
            cur = (j - i) * min(height[i], height[j]);
            if (cur > max)
                max = cur;
            if (height[i] > height[j])
                --j;
            else
                ++i;
        }

        return max;
    }
};

int main (int argc, char *argv[]) {
    
    Solution s;
    // vector<int> v = {1,8,6,2,5,4,8,3,7};
    vector<int> v = {3,5, 1};
    cout << s.maxArea(v) << endl;
    return 0;
}

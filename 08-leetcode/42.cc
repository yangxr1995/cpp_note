#include <climits>
#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    int trap(vector<int>& height) {
        int right, left;
        int right_max, left_max;

        right = height.size() - 1;
        left = 0;
        int sum = 0;
        right_max = height[right];
        left_max = height[left];
        while (left < right) {
            if (height[left] < height[right]) {
                left++;
                if (height[left] < left_max) {
                    sum += left_max - height[left];
                }
                else {
                    left_max = height[left];
                }
            }
            else {
                right--;
                if (height[right] < right_max) {
                    sum += right_max - height[right];
                }
                else {
                    right_max = height[right];
                }
            }
        }
        return sum;
    }
};

int main (int argc, char *argv[]) {

    Solution s;
    // vector<int> v = { 0,1,0,2,1,0,1,3,2,1,2};
    vector<int> v = {4,2,0,3,2,5};
    // vector<int> v = {5,4,1,2};
    // vector<int> v = {8,7,2,5,6};
    cout << s.trap(v) << endl;

    return 0;
}

#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int>arr;

	arr.push_back(1);

	for (int i = 0; i < 10; i++)
		arr.push_back(i);

	for (vector<int>::iterator it = arr.begin(); it != arr.end(); ) {
		if (*it % 2 == 0) {
			it = arr.erase(it);
			arr.insert(it, 3);
		}
		else {
			++it;
		}
	}

	for (auto it = arr.begin(); it != arr.end(); ++it)
		cout << *it << " ";
	cout << endl;

	for (int i = 0; i < arr.size(); ++i)
		cout << arr[i] << " ";
	cout << endl;

	for (int i : arr)
		cout << i << " ";
	cout << endl;

	return 0;
}

#include <iostream>
#include <vector>
using namespace std;

void vectorTest()
{
	vector<int> container;
	for (int i = 0; i < 10; i++)
	{
		container.push_back(i);
	}

	vector<int>::iterator iter;
	for (iter = container.begin(); iter != container.end(); iter++)
	{
		if (*iter > 3)
			container.erase(iter);
	}

	for (iter = container.begin(); iter != container.end(); iter++)
	{
		cout<<*iter<<endl;
	}
}

int main()
{
	vectorTest();
	return 0;
}

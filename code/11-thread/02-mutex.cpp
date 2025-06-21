#include <iostream>
#include <mutex>
#include <thread>
#include <list>
using namespace std;


int sum_car = 100;

std::mutex mtx;

void sell_car_mtx(int i)
{
	while (1) {
		mtx.lock();
		if (sum_car <= 0) {
			mtx.unlock();
			break;
		}
		cout << i << " sell car" << sum_car << endl;
		sum_car--;
		mtx.unlock();
	}
}

void sell_car_lock_guard(int i)
{
	while (1) {
		{
			// 和 scoped_ptr 一样，没有拷贝构造
			// 出作用域析构自动解锁
			lock_guard<mutex> lck(mtx);
			if (sum_car <= 0)
				break;
			cout << i << " sell car" << sum_car << endl;
			sum_car--;
		}
	}
}

void sell_car_unique_lock(int i)
{
		while (1) {
		{
			// 和 unique_ptr 一样
			// 有右值拷贝构造，可用于函数返回值传递锁
			unique_lock<mutex> lck(mtx);
			if (sum_car <= 0)
				break;
			cout << i << " sell car" << sum_car << endl;
			sum_car--;
		}
	}

}

int main()
{
	cout << "--------------- 02-mutex.cpp --------------" << endl;
	list<thread> l;

	for (int i = 0; i < 10; ++i)
		l.push_back(thread(sell_car_unique_lock, i));

	for (auto& t : l)
		t.join();

	return 0;
}

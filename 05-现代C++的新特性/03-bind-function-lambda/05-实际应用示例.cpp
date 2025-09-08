#include <functional>
#include <vector>
#include <thread>
#include <iostream>
using namespace std;

class thread_item {
	public:
		// 使用 function 定义函数对象，和函数指针很像
		thread_item(function<void (void)> f) 
		:_f(f){}

		void start() {
			_f();
		}
	private:
		function<void (void)> _f;	
};

class thread_pool {
	public:
		thread_pool() {
		}
		void start(unsigned int nb = 10) {
			for (int i = 0; i < nb; i++) {
				// 1. 将成员函数转换为函数对象传递给其他对象，使用 bind
				threads.push_back(new thread_item(bind(&thread_pool::run_in_thread, this, i)));
			}

			for (auto it = threads.begin(); it != threads.end(); ++it) {
				// 2. threads 的元素为 thread_item * 的 vector，所以 it 为指向 thread_item * 的迭代器
				(*it)->start();
			}
		}
		~thread_pool() {
			for (auto it = threads.begin(); it != threads.end(); ++it) {
				delete *it;
			}
		}
	private:
		vector<thread_item *> threads;
		void run_in_thread(unsigned int id) {
			cout << "run thread " << id << endl;	
		}
};

int main()
{
	thread_pool tp;

	tp.start();

	return 0;
}

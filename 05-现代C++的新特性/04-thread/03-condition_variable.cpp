#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>

using namespace std;

class queue_mtx {
	public:
		int get() {
			int val;

			unique_lock<mutex> lck(_m);
			while (_q.empty()) {
				cout << "get wait" << endl;
				// lck 只能为 unique_lock ,因为他有右值拷贝可以传递给 condition_variable
				// condition_variable 先释放锁 ，再在条件变量上等待
				// 当被唤醒时，_cv 会重新获得锁
				//
				// lock_guard 就不行，因为他没有右值拷贝
				_cv.wait(lck);
			}
			val = _q.front();
			_q.pop();
			// 唤醒所有等待的线程
			_cv.notify_all();
			cout << "get " << val << endl;
			return val;
		}

		void put(int val) {
			unique_lock<mutex> lck(_m);
			while (!_q.empty()) {
				cout << "put wait" << endl;
				_cv.wait(lck);
			}
			_q.push(val);
			_cv.notify_all();
			cout << "put " << val << endl;
		}

	private:
		queue<int> _q;
		condition_variable _cv;
		mutex _m;
};

void thread_handler1(queue_mtx *q)
{
	while (1) {
		q->put(rand()%100);
		sleep(1);
	}
}

void thread_handler2(queue_mtx *q)
{
	while (1) {
		int v = q->get();
		sleep(2);
	}
}

int main()
{
	cout << "--------------- 03-condition_variable.cpp --------------" << endl;
	queue_mtx q_mtx;

	std::thread t1(thread_handler1, &q_mtx);
	thread t2(thread_handler2, &q_mtx);
//
	t1.join();
	t2.join();
//
	return 0;
}

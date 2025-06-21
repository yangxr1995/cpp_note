#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

/*
 * thread/mutex/condition_variable
 * lock_quard/unique_lock
 * atomic 
 * sleep_for
 */

void thread_handle1()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));

	cout << "hello thread1" << endl;
}

void thread_handle2(int i)
{
	std::this_thread::sleep_for(std::chrono::seconds(i));
	cout << "thread2 done" << endl;
}

int main()
{
	cout << "--------------- 01-thread.cpp --------------" << endl;

	std::thread t(thread_handle1);

	std::thread t1(thread_handle2, 2);

#if 0
	t.join();
	t1.join();
#endif
	t.detach();
	t1.detach();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	cout << "main thread done!" << endl;

	return 0;	
}


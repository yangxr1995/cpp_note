#include <bits/c++config.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <ostream>
using namespace std;

/*
 * 1. new和malloc区别
 *    a. malloc返回void *，new返回对于类型的指针
 *    b. 对于类类型,new会调用构造
 *    c. new时可以进行元素初始化
 *    d. 可以定位new
 */

/*
 * 2. new t和 delete[] 可以混用吗？
 *    基础类型可以混用
 *    类类型不能混用，因为类类型要进行析构，
 *    编译器为了知道析构的对象数量，在构造 new vector<int>[5] 时，
 *    首先分配4字节存放数组元素数量5，再分配5个vector<int>的内存
 *    再对每个元素进行定位new，调用其构造函数。
 *    返回给用户的地址不是分配的首地址（存放数组元素数量的内存的地址）
 *    而是数组首元素的地址。
 *
 *    当delete[] xx时，编译器会对xx地址想后偏移4字节得到数组元素的长度，
 *    并根据长度对xx开始的元素数组依次调用析构函数，
 *    再释放整个内存
 */


/*
 * 3. 重载new
 *    常用于实现对象持，示例中QueueItem会被频繁分配
 *    释放，所以可以预先分配，之后从池中获取。
 *    需要注意的是 new 的参数决定了链接的函数，也就
 *    决定了在哪个类中定义重载的new
 *
 * 需要注意 new delete 时编译器加的代码
 *
 * void *operator new(size_t sz);
 * 如此重载后，调用new处的代码调用流程为
 * operator new(size_t sz) // 首先分配空间
 * 构造基类，基类构造顺序为在子类中的声明顺序
 * 构造组合类，构造顺序为声明顺序
 * 构造自己
 *
 * void operator delete(void *addr);
 * 当重载delete后，调用delete处的代码调用流程为
 * 析构自己
 * 析构组合类
 * 析构基类
 * 调用 operator delete(void *), 可用于释放空间
 *
 * void *operator new(size_t sz, void *addr)
 * placement new 重载后，调用new处的代码调用流程为
 * 构造基类，基类构造顺序为在子类中的声明顺序
 * 构造组合类，构造顺序为声明顺序
 * operator new(size_t sz, void *addr)
 * 构造自己
 */

template<typename T>
class Queue {
	public:
		Queue() {
			_first = nullptr;
			_last = nullptr;
		}
		void push(const T &a) {
			QueueItem *n;			

			n->_next = _first;
			if (_first)
				_first->_prev = n;
			_first = n;
			if (_last == nullptr)
				_last = n;
		}
		T pop() {
			T ret;
			QueueItem *tmp;
			if (_last == nullptr)
				throw "Queue empty";
			ret = _last->_data;
			tmp = _last;
			_last = _last->_prev;
			if (_last == nullptr)
				_first = nullptr;

			delete tmp;

			return ret;
		}

		struct QueueItem {
			QueueItem() {}
			QueueItem(const T &data)
			:_data(data), _next(nullptr), _prev(nullptr) {
				cout << "QueueItem construct" << endl;
			}
			~QueueItem() {
				cout << "~QueueItem destruct" << endl;
			}

			// placement new
			// 在分配了内存之后，调用自己的构造之前调用placement new
			void *operator new(size_t sz, QueueItem *addr) {
				cout << "QueueItem placement new" << endl;	
				return addr;
			}

			// 普通new
			// 用于分配内存，在所有构造之前被调用
			void *operator new(size_t sz) {
				cout << "QueueItem operator new" << endl;
				void *n;

				if (_qlist_cache != nullptr) {
					n = _qlist_cache;	
					_qlist_cache = _qlist_cache->_next;
				}
				else {
					cout << "malloc" << endl;
					n = malloc(sz);
				}

				return n;
			}

			void operator delete(void *addr) {
				cout << "QueueItem operator delete " << endl;
				QueueItem *n = (QueueItem *)addr;
					
				n->_next = _qlist_cache;
				_qlist_cache = n;
			}

			T _data;
			QueueItem *_next;
			QueueItem *_prev;
			static QueueItem *_qlist_cache;
		};

	private:
		QueueItem *_first;
		QueueItem *_last;
};

// 1. 使用typename声明T为类型，之后才可以用
template<typename T>
typename Queue<T>::QueueItem g_a;

// 1. typename只能管下面一个表达式，所以新的表达式需要重新声明T
// 2. Queue<T>::QueueItem 可能为熟悉，也可以为类型，需要用typename强制指定为类型
template<typename T>
typename Queue<T>::QueueItem *Queue<T>::QueueItem::_qlist_cache = nullptr;

class Student {
	public:
		Student(unsigned int age = 0)
		:_age(age){
			cout << "Student construct" << _age << endl;
		}
		Student(const Student &stu) 
		:_age(stu._age){
			cout << "Student copy construct" << _age << endl;	
		}
		~Student() {
			cout << "Student destruct" << endl;
		}
		friend ostream &operator<<(ostream &out, const Student &s);
	private:
		int _age;
};

ostream &operator<<(ostream &out, const Student &s)
{
	out << "Student age "  << s._age << " ";
	return out;
}

int test() 
{
	Queue<Student> q;	

	for (int i = 0; i < 10; i++) {
		cout << "push begin" << endl;
		q.push(Student(i));
		cout << "push end" << endl;
	}

	for (int i = 0; i < 10; i++) {
		cout << "pop begin" << endl;
		cout << q.pop() << " ";
		cout << "pop end" << endl;
	}

	for (int i = 0; i < 10; i++) {
		cout << "push begin" << endl;
		q.push(i);
		cout << "push end" << endl;
	}

	return 0;
}

int main()
{
	cout << "test begin" << endl;
	test();
	cout << "test end" << endl;
	return 0;
}

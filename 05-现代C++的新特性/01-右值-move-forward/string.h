#include <alloca.h>
#include <cstring>
#include <iostream>
#include <istream>
#include <ostream>
#include <sched.h>
#include <utility>
using namespace std;

/*
 * 代码改进建议
 * 1. 赋值运算符的异常安全性
 * 当前的赋值运算符实现没有考虑到异常安全性。如果new char[]因为任何原因抛出异常，当前对象将处于破坏状态。
 * 
 * 一个更安全的做法是使用拷贝和交换模式（Copy-and-Swap Idiom），这不仅解决了异常安全性问题，也解决了自赋值的问题。
 * 
 * 2. 移动构造函数和移动赋值运算符的实现
 * C++11 引入了移动语义，允许资源的所有权从一个对象转移到另一个。对于资源管理密集的类（如自己管理内存的类），实现移动构造函数和移动赋值运算符可以显著提高性能。
 * 
 * 3. std::istream &operator>>(std::istream &in, String &s)的改进
 * 这个函数的当前实现简单地使用>>操作符读取istream到_str，这是不安全的，因为它不考虑_str的大小。这可能会导致缓冲区溢出。
 * 
 * 4. 使用标准库函数和类型
 * 使用C标准库的strlen，strcpy和strcat函数是合适的，但在C++中，考虑使用std::copy，std::fill_n等STL算法和std::vector<char>或std::unique_ptr<char[]>作为底层存储，可以使代码更加安全和现代化。
 */

class String {
	public:
		String(const char *str = nullptr) {
            cout << __PRETTY_FUNCTION__ << endl;
			if (str == nullptr)
				str = "";
			_str = new char[strlen(str) + 1]{0};
			strcpy(_str, str);
		}

		String(const String &s) {
            cout << __PRETTY_FUNCTION__ << endl;
			_str = new char[strlen(s._str) + 1]{0};
			strcpy(_str, s._str);
		}

		String(String &&s) {
            cout << __PRETTY_FUNCTION__ << endl;
			_str = s._str;
			s._str = nullptr;
		}

		~String() {
            cout << __PRETTY_FUNCTION__ << endl;
			if (_str != nullptr)
				delete[] _str;
		}

		String &operator=(const String &s) {
            cout << __PRETTY_FUNCTION__ << endl;
			delete _str;
			_str = new char [strlen(s._str) + 1] {0};
			strcpy(_str, s._str);
			return *this;
		}

		String &operator=(String &&s) {
            cout << __PRETTY_FUNCTION__ << endl;
			delete _str;
			_str = s._str;
			s._str = nullptr;
			return *this;
		}

		class iterator {
			public:
				iterator(char *p)
				:_p(p) {}

				iterator operator++() {
					_p++;
					return *this;
				}

				bool operator!=(const iterator &i) const {
					return _p != i._p;	
				}
				
				/*
				 * 返回引用以支持
				 *   *it = '1';
				 */
				char &operator*() {
					return *_p;	
				}

				// 支持const迭代器
				const char &operator*() const {
					return *_p;	
				}

			private:
				char *_p;
		};

		iterator begin() const {
			return iterator(_str);
		}

		iterator end() const {
			return iterator(_str + strlen(_str));
		}

		// 不能做类方法，否则下面表达式错误
		// s4 = "bbb" + s3;
		friend String operator+(const String &s1, const String &s2);
		friend ostream &operator<<(ostream &out, const String &s);
		friend istream &operator>>(istream &in, String &s);

		const char *c_str() const {
			return _str;
		}
	private:
		char *_str;
};

inline ostream &operator<<(ostream &out, const String &s) {
	out << s._str;
	return out;
}

inline String operator+(const String &s1, const String &s2) {
    cout << __PRETTY_FUNCTION__ << endl;
	// String::String(const char *s)
	String tmp;
	tmp._str = new char[strlen(s1._str) + strlen(s2._str) + 1]{0};
	strcpy(tmp._str, s1._str);
	strcat(tmp._str, s2._str);
	// 匿名变量 String::String(String &&)
	// String::~String()
//	return std::move(tmp);

	// 匿名变量 String::String(const String &s)
	// String::~String()
	return tmp;
}

inline istream &operator>>(istream &in, String &s) {
	in >> s._str;
	return in;
}


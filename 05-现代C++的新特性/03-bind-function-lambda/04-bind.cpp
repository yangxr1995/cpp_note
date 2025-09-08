#include <cstdlib>
#include <ctime>
#include <functional>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

template<typename T>
void show_containter(T &c)
{
	for (auto it = c.begin(); it != c.end(); ++it)
		cout << *it << " ";	
	cout << endl;
}

void bind2nd_demo()
{
	cout << "------------- 演示bind2nd ------------" << endl;
	vector<int> v;
	v.reserve(20);

	srand(time(nullptr));
	for (int i = 0; i < 20; i++)
		v.push_back(rand()%100);

	show_containter(v);

	sort(v.begin(), v.end(), less<int>());

	show_containter(v);

	/*
	 * find_if  preid 是一元函数对象，使用 bind2nd/ bind1st 将二元转换为一元
	 */
	auto it = find_if(v.begin(), v.end(), bind2nd(greater<int>(), 70));
	if (it == v.end()) {
		cout << "没有比70大的元素" << endl;
	}
	else {
		cout << "找到比70大的元素 : " <<  *it << endl;
		v.insert(it, 100);
	}

	show_containter(v);

}

template<typename Compare, typename T>
class _mybind2nd {
	public:
		_mybind2nd(const Compare &comp, const T &arg2)
			:_comp(comp), _arg2(arg2) {}
		bool operator()(const T &arg1) {
			return _comp(arg1, _arg2);
		}
	private:
		Compare _comp;
		T _arg2;
};

// 将 mybind2nd 定义为模板函数，而非模板类，
// 是因为 模板函数 有类型自动推演，
// 根据推演的类型再调用函数对象的构造
template<typename Compare, typename T>
_mybind2nd<Compare, T> mybind2nd(Compare comp, T arg2)
{
	return _mybind2nd<Compare, T>(comp, arg2);
}

void bind2nd_imple()
{
	cout << "------------ 实现bind2nd -------------" << endl;
	vector<int> v;
	v.reserve(20);

	srand(time(nullptr));
	for (int i = 0; i < 20; i++)
		v.push_back(rand()%100);

	show_containter(v);

	sort(v.begin(), v.end(), less<int>());

	show_containter(v);

	/*
	 * find_if  preid 是一元函数对象，使用 bind2nd/ bind1st 将二元转换为一元
	 */
	auto it = find_if(v.begin(), v.end(), mybind2nd(greater<int>(), 70));
	if (it == v.end()) {
		cout << "没有比70大的元素" << endl;
	}
	else {
		cout << "找到比70大的元素 : " <<  *it << endl;
		v.insert(it, 100);
	}

	show_containter(v);
}

void hello()
{
	cout << "hello world" << endl;
}

void print(string str)
{
	cout << str << endl;
}

int sum(int a, int b)
{
	return a + b;
}

void bind_demo()
{
	cout << "------------- 演示bind ------------" << endl;
	bind(hello)();
	bind(print, "hello world")();
	cout << bind(sum, 1, 2)() << endl;

	// 占位符
	// 使用一个占位符，占用第一个参数
	cout << bind(sum, placeholders::_1, 2)(3) << endl;

	cout << bind(sum, placeholders::_1, placeholders::_2)(3, 2) << endl;

	function<int (int, int)> func1 = bind(sum, placeholders::_1, placeholders::_2);
	cout << func1(1, 2) << endl;

	// 占位符从 placeholders::_1 开始依次使用
	// 即下面语句不能写成
	// function<int (int)> func2 = bind(sum, 2, placeholders::_2);
	function<int (int)> func2 = bind(sum, 2, placeholders::_1);
	cout << func2(2) << endl;

}

int main()
{
	//bind2nd_demo();
//	bind2nd_imple();

	bind_demo();

	return 0;
}

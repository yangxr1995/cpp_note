#include <iostream>
#include <string>
#include <map>
#include <ostream>
#include <utility>
using namespace std;

class Stu {
	public:
		Stu(int id = 0, const char *name = "") 
			:_id(id), _name(name) {}
		friend ostream &operator<<(ostream &out, const Stu &stu);
	private:
		int _id;
		string _name;
};

ostream &operator<<(ostream &out, const Stu &stu)
{
	out << stu._id << ", " << stu._name << endl;
	return out;
}

int main()
{
	map<int, Stu>stu_map;
	Stu *pstu;

	pstu = new Stu(12, "aa");
	// map 不需要定义 operator< 因为他使用 key 的operator<
	// 这里key为 int ，int 的< 是存在的
	// 由map根据key值，计算位置，插入元素
	stu_map.insert(make_pair(12, Stu(12, "aa")));
	stu_map.insert(make_pair(13, Stu(13, "bb")));
	stu_map.insert(make_pair(10, Stu(10, "cc")));

	// 由程序员指定插入位置
	stu_map[1] = Stu(111, "xxx");

	// 中序遍历红黑树
	for (const pair<int, Stu>pos : stu_map)
		cout << pos.second;
	cout << endl;
	
	for (map<int, Stu>::iterator it = stu_map.begin(); it != stu_map.end(); ++it)
		cout << it->second;
	cout << endl;

	/* 
	 * class iterator : public const_iterator
	 * 所以 iterator --> const_iterator
	 */
	for (map<int, Stu>::const_iterator it = stu_map.begin(); it != stu_map.end(); ++it)
		cout << it->second;
	cout << endl;

	for (map<int, Stu>::reverse_iterator it = stu_map.rbegin(); it != stu_map.rend(); ++it)
		cout << it->second;
	cout << endl;

	for (map<int, Stu>::const_reverse_iterator it = stu_map.rbegin(); it != stu_map.rend(); ++it)
		cout << it->second;
	cout << endl;

	return 0;
}

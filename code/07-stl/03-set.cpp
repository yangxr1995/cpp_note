#include <iostream>
#include <ostream>
#include <string>
#include <set>
using namespace std;

class Stu {
	public:
		Stu(int id = 0, string name = "")
			:_id(id), _name(name) {} 
		friend ostream &operator<<(ostream &out, const Stu &s);
		// 对于顺序关联容器，需定义 < 的常方法
		bool operator<(const Stu &s) const {
			return this->_id < s._id;
		}
	private:
		string _name;
		int _id;
};

ostream &operator<<(ostream &out, const Stu &s)
{
	out << s._id << " " << s._name;
	return out;
}

int main()
{
	set<Stu> stu_set;

	stu_set.insert(Stu(70, "aa"));
	stu_set.insert(Stu(20, "bb"));
	stu_set.insert(Stu(10, "cc"));
	stu_set.insert(Stu(30, "dd"));
	stu_set.insert(Stu(20, "xx")); // 不支持重复，重复添加的元素会被忽略

	for (Stu pos : stu_set)
		cout << pos << endl;

	return 0;
}

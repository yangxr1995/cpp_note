#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <queue>
#include <vector>
using namespace std;


class Stu {
    public:
        string name;
        int age;
        friend ostream &operator<<(ostream &out, const Stu &s);
        bool operator<(const Stu &s) const {
            return age < s.age;
        }
};

ostream &operator<<(ostream &out, const Stu &s)
{
    out << s.name << " " << s.age;
    return out;
}

int main (int argc, char *argv[]) {
    auto cmp = [] (const Stu &s1, const Stu &s2) -> bool { return s1.age > s2.age; };
    // priority_queue<Stu, deque<Stu>, decltype(cmp)> q(cmp);
    priority_queue<Stu> q;
    vector<Stu> v = {
        {.name = "cc", .age = 11},
        {.name = "aa", .age = 33},
        {.name = "bb", .age = 22},
    };
    for (const auto & stu : v) {
        q.push(stu);
    }
    while (!q.empty()) {
        cout << q.top() << endl;
        q.pop();
    }
    cout << endl;
    
    return 0;
}


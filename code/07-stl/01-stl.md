# STL
## 概览
- 顺序容器
    - vector
    - deque
    - list
- 容器适配器
    - stack
    - queue
    - priority_queue
- 关联容器
    - 无序关联容器
        - unordered_set
        - unordered_multiset
        - unordered_map
        - unordered_multimap
    - 有序关联容器
        - set
        - multiset
        - map
        - multimap
- 近容器
    - 数组
    - string
    - bitset
- 迭代器
    - iterator
    - const_iterator
    - reverse_iterator
    - const_reverse_iteraotr
- 函数对象
    - greater
    - less
- 泛型算法
    - sort
    - find
    - find_if
    - binary_search
    - for_earch


## 非关联容器
### vector

     1. vector 的扩容是二倍
         vector
            ptr ──────► nullptr
            cap=0
            size=0

         vector        ┌─┐
            ptr ──────►│0│
            cap=1      └─┘
            size=1
 
         vector        ┌───┐
            ptr ──────►│0 1│
            cap=2      └───┘
            size=2
 
         vector        ┌───────┐
            ptr ──────►│0 1 2  │
            cap=4      └───────┘
            size=3

         vector        ┌───────┐
            ptr ──────►│0 1 2 3│
            cap=4      └───────┘
            size=4

         vector        ┌────────────────┐
            ptr ──────►│0 1 2 3 5       │
            cap=8      └────────────────┘
            size=5
 
     2. vector尾部增删元素效率为O(1), 其他为O(n)

         vector        ┌────────────────┐                   vector        ┌────────────────┐
            ptr ──────►│0 1 2 3 5       │                      ptr ──────►│0 1 2 3 5 6     │
            cap=8      └────────────────┘                      cap=8      └────────────────┘
            size=5                                             size=5                       

         vector        ┌────────────────┐需要对其他元素移位 vector        ┌────────────────┐ 
            ptr ──────►│0 1 2 3 5       │     ────────►        ptr ──────►│0 1 6 2 3 5     │ 
            cap=8      └────────────────┘                      cap=8      └────────────────┘ 
            size=5                                             size=5                        

                                                                                                                                                                                                   
底层为动态数组，每次以原来大小进行2倍扩容
* 动态数组
* 二倍扩容
* first指针指向数组首元素，导致只有push_back的效率为O(1)，其他效率都为O(n)

```cc
vector<int> v;

// 1. 增
v.push_back(1); // 尾部添加元素
v.insert(v.begin(), 2); // 迭代器指定位置添加元素

// 2. 删
v.pop_back(); // 尾部删除元素
v.erase(v.begin()); // 删除迭代器指定的元素

// 3. 查询/ 修改
v[1] = 1;
// find, for_each
// foreach

// 4. 常用
v.size(); // 有效元素数量
v.empty();
// v.reserve(20); 预留空间
// v.resize(20); // 修改size
vector<int> v2;
v.swap(v2); // 两个容器交换元素
```

示例
```cc
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

int main (int argc, char *argv[]) {
    vector<int> v;

    v.reserve(20);
    v.resize(20);

    cout << v.capacity() << endl;
    cout << v.size() << endl;

    for (int i = 0; i < 10; i++) {
        v.push_back(rand() % 100 + 1);
    }

    int size = v.size();
    for (int i = 0; i < size; i++) {
        cout << v[i] << " ";
    }
    cout << endl;

    for (auto it = v.begin(); it != v.end(); ++it) {
        if (*it%2 == 0) {
            it = v.erase(it);
            --it;
    }
    }

    for (auto it = v.begin(); it != v.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    for (auto it = v.begin(); it != v.end(); ++it) {
        if (*it % 2 != 0) {
            v.insert(it, *it + 1);
            ++it;
        }
    }

    for (auto i : v) {
        cout << i << " ";
    }
    cout << endl;


    return 0;
}
```

### deque

                               first
                               │
        ┌──┐                   │
        │  │                   ▼
        ├──┤    ┌──────────────┬───────────────┐ 
        │  ├───►│............. │               │ 
        │  │    └──────────────┴───────────────┘ 
        ├──┤    ┌──────────────────────────────┐
        │  ├───►│............................. │
        │  │    └──────────────────────────────┘
        ├──┤    ┌───────┬──────────────────────┐ 
        │  ├───►│...... │                      │ 
        │  │    └───────┴──────────────────────┘ 
        ├──┤            ▲
        └──┘            │ last

* 底层是二维数组
* 4K扩容
* 使用两个指针first指针和last指针，指向已占用数组的首尾
* first last从空闲空间的中间开始分配
* 当二维空间消耗完了，就对一维数组进行扩容
* push_back 和 push_front 效率为O(1)


### list 
双向循环链表

* insert O(1)
* 访问[] O(n)

### vector deque list 对比
- vector vs deque
    - 扩容
        - vector 是基于连续的数组，经常扩容，每次扩容都会进行对象的析构和构造
        - deque 基于不连续的数组，扩容时，不会进行对象的深拷贝
        - list 是离散的，不涉及扩容
    
    - 中间删除
        - deque 删除中间元素，需要进行元素移动，如果有多个二维数组，则需要跨数组移动，效率低
        - vector删除中间元素，需要进行元素移动，但是由于内存一定连续，所以效率相对高


- list vs vector
    - 增删
        - list O(1)
        - vector O(n)
    - 访问
        - list O(n)
        - vector O(1)
 
# 关联容器

关联容器的特点是增删不能指定位置，直接给存储的值。

## 无序关联容器
底层使用hash表

### unordered_set 

```cc
#include <cstdlib>
#include <iostream>
#include <unordered_set>
using namespace std;

int main (int argc, char *argv[]) {

    // set<int> s;    // 不可重复, 有序
    unordered_multiset<int> s;    // 不可重复, 无序
    // multiset<int> s;  // 可重复 , 无序

    for (int i = 0; i < 20; ++i) {
        s.insert(rand()%10 + 1);   // 管理容器不指定插入位置，只指定插入值
    }
    cout << s.size() << endl;
    cout << s.count(7) << endl;   // 打印对于值的元素个数

    // 迭代器不能定义为常引用
    for (unordered_set<int>::iterator it = s.begin() ; it != s.end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;

    for (auto it = s.begin(); it != s.end(); ) {
        if (*it == 4) {
            it = s.erase(it); // erase(iterator) 或者 erase(key)
        }
        else {
            ++it;
        }
    }

    auto it = s.find(5); // 找不到返回end
    if (it == s.end()) {
        cout << "can't find 5" << endl;
    }

    for (int i  : s) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
```

## unordered_map

和set 类似，但是存的是 pair
```cc
struct pair {
    private:
        T1 first;  // key
        T2 second; // value
}
```

### 示例
```cc
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
using namespace std;

int main (int argc, char *argv[]) {

    unordered_map<int, string> m;

    m.insert(make_pair(111, "aaa"));
    m.insert({222, "bbb"});
    m.insert({333, "ccc"});
    cout << m.size() << endl;

    // operator[]
    cout << m[111] << endl; // 如果key存在，则返回value，否则增加key，value为默认构造 

    m[444]; // 会增加key为444的元素，value为默认值

    cout << m.size() << endl;
    m.erase(444);

    m[444] = "ddd";
    cout << m.size() << endl;

    auto it = m.find(444);
    if (it != m.end()) {
        cout << "key : " << it->first << " value : " << it->second << endl;
    }

    return 0;
}
```

## 有序关联容器
有序关联容器需要定义类的`operator<`
```cc
#include <set>
#include <string>
#include <iostream>
using namespace std;

class Stu {
    public:
        Stu(int id, const string &name)
        : _id(id), _name(name) {}
        bool operator<(const Stu &s) const {
            return _id < s._id;
        }
    private:
        int _id;
        string _name;

    friend ostream &operator<<(ostream &out, const Stu &s);
};

ostream &operator<<(ostream &out, const Stu &s) {
    out << "id : " << s._id << " name : " << s._name;
    return out;
}

int main (int argc, char *argv[]) {

    set<Stu> s;
    s.insert(Stu(2, "bb"));
    s.insert(Stu(3, "cc"));
    s.insert(Stu(1, "aa"));
    s.insert(Stu(4, "dd"));

    for (const Stu &s  : s) {
        cout << s << endl;
    }
    
    return 0;
}
```

# 迭代器
```cpp
// 正向迭代器
  for (map<int, Stu>::iterator it = stu_map.begin(); it != stu_map.end(); ++it)
      cout << it->second;
  cout << endl;

// const正向迭代器
  /*
   * class iterator : public const_iterator
   * 所以 iterator --> const_iterator
   */
  for (map<int, Stu>::const_iterator it = stu_map.begin(); it != stu_map.end(); ++it)
      cout << it->second;
  cout << endl;

// 反向迭代器
  for (map<int, Stu>::reverse_iterator it = stu_map.rbegin(); it != stu_map.rend(); ++it)
      cout << it->second;
  cout << endl;

// const反向迭代器
  for (map<int, Stu>::const_reverse_iterator it = stu_map.rbegin(); it != stu_map.rend(); ++it)
      cout << it->second;
  cout << endl;
```


# 容器适配器
## 原理
```cc
#include <cstdio>
#include <deque>
#include <iostream>
using namespace std;

template <typename T, typename Container=deque<T>>
class mystack {
    public:
        void push(const T &a) {
            _c.push_back(a);
        }
        void pop() {
            _c.pop_back();
        }
        T top() const {
            return _c.back();
        }
        bool empty() const {
            return _c.empty();
        }
    private:
        Container _c;
};


template <typename T, typename Container=deque<T>>
class myqueue {
    public:
        void push(const T &a) {
            _c.push_front(a);
        }
        void pop() {
            _c.pop_back();
        }
        T top() const {
            return _c.back();
        }
        bool empty() const {
            return _c.empty();
        }
    private:
        Container _c;
};

int main (int argc, char *argv[]) {

    // mystack<int> s;
    myqueue<int> s;
    s.push(1);
    s.push(2);
    s.push(3);

    while (!s.empty()) {
        cout << s.top() << endl;
        s.pop();
    }
    cout << endl;
    
    return 0;
}
```

## C++提供的重要容器适配器
stack : 底层 deque
queue : 底层 deque
priority_queue : 底层 vector, 因为priority_queue算法是大根堆，依赖数组索引实现，所以必须要连续的内存

# 泛型算法



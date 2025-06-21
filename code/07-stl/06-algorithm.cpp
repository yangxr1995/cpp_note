#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

// Forward declaration of the print_info class
template <typename T>
class print_info;

// Declare the operator<< as a template function
template <typename T>
ostream & operator<<(ostream &out, print_info<T> &p);

template <typename T>
class print_info{
    public:
        print_info()
        : is_init(false), m_sum(0), m_max(0), m_min(0)
        {
            cout << __PRETTY_FUNCTION__ << endl;
        }

        print_info(const print_info &p) 
        : is_init(p.is_init), m_sum(p.m_sum), m_max(p.m_max), m_min(p.m_min)
        {
            cout << __PRETTY_FUNCTION__ << endl;
        }

        void operator() (const T &a) {
            if (is_init == false) {
                is_init = true;
                m_min = a;
                m_max = a;
            }
            else {
                if (a > m_max)
                    m_max = a;
                if (a < m_min)
                    m_min = a;
            }
            m_sum += a;
        }

        print_info<T> &operator=(const print_info<T> &a) {
            cout << __PRETTY_FUNCTION__ << endl;
            return *this;
        }

        print_info<T> &operator=(print_info<T> &&a) {
            cout << __PRETTY_FUNCTION__ << endl;
            return *this;
        }

    private:
        T m_sum;
        T m_max;
        T m_min;
        bool is_init;

        friend ostream & operator<< <>(ostream &out, print_info<T> &p); // <> : 让编译器知道此这是模板函数的声明，而非模板类中的普通函数
        // friend ostream & operator<<(ostream &out, print_info<T> &p); // 编译器会认为这是模板类中的普通函数,导致链接错误
};

template <typename T>
ostream & operator<<(ostream &out, print_info<T> &p)
{
    out << "min : " << p.m_min << ", max : " << p.m_max << ", sum : " << p.m_sum << endl;
    return out;
}

template <typename T>
T test(T a) {
    cout << __PRETTY_FUNCTION__ << endl;
    return a;
}

template<typename T>
class Encrypt {
};

template<>
class Encrypt<string> {
    public:
        string operator()(const string &s) {
            string tmp = s;
            for (string::iterator it = tmp.begin(); it != tmp.end(); ++it) {
                *it = *it + 1;
            }
            return tmp;
        }
};


int main (int argc, char *argv[]) {


    // 1. for_each
    {
        vector<int> v = {1, 2, 3, 4, 5};
        // 遍历集合，调用 __f(*i)
        // for_each(_InputIterator __first, _InputIterator __last, _Function __f)
        print_info<int> p = std::for_each(v.begin(), v.end(), print_info<int>());
        // cout << p << endl;
    }

    // 2. find
    {
        int a[] = {2, 3, 4, 2, 3, 4, 5, 12, 11};
        int *pbegin = a;
        int *pend = a + sizeof(a)/sizeof(int);
        // 找 *i = __val
         //   find(_InputIterator __first, _InputIterator __last,
         // const _Tp& __val)
        int *p = find(pbegin, pend, 3);
        if (p != pend)
            cout << *p << endl;

        // 找 __pred(i) 为真
        // find_if(_InputIterator __first, _InputIterator __last,
        //  _Predicate __pred)
        p = find_if(pbegin, pend, bind(greater<int>(), placeholders::_1, 4));
        if (p != pend)
            cout << *p << endl;


        int b[] = {10, 12};
        // [__first1, __last1) 中找 [__first2, __last2)中的匹配值
        // find_first_of(_InputIterator __first1, _InputIterator __last1,
        // _ForwardIterator __first2, _ForwardIterator __last2)
        p = find_first_of(pbegin, pend, b, b + sizeof(b)/sizeof(int));
        if (p != pend)
            cout << *p << endl;
        else
            cout << "no" << endl;

        // 在 [pbegin, pend) 区间中找 *i == *(i + 1) ，即相邻相等的元素，返回首个找到的迭代器
        p = adjacent_find(pbegin, pend);
        if (p != pend)
            cout << *p << endl;

        int c[] = {3, 4};

        // 找最后一个匹配的子序列
        // 在[__first1, __last1) 区间中使用匹配序列 [__first2, __last2) 找到最后一个匹配的序列
        // find_end(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
        //   _ForwardIterator2 __first2, _ForwardIterator2 __last2)
        p = find_end(pbegin, pend, c, c + sizeof(c)/sizeof(*c));
        if (p != pend)  {
            for (; p != pend; ++p) {
                cout << *p << " ";
            }
            cout << endl;
        }
        else
            cout << "no" << endl;

        // 找首个匹配的子序列
        p = search(pbegin, pend, c, c + sizeof(c)/sizeof(*c));
        if (p != pend) {
            for (; p != pend; ++p) {
                cout << *p << " ";
            }
            cout << endl;
        }
        else
            cout << "no" << endl;

    }

    // 3. count
    {
        bitset<7> b(string("0011001"));
        array<int, 7> a;
        for (int i = 0; i < b.size(); i++) {
            a[i] = b[i];
        }
        // 返回 *i == __value 的元素数量
        // count(_InputIterator __first, _InputIterator __last, const _Tp& __value)
        cout << count(a.begin(), a.end(), 1) << endl;
        cout << b.count() << endl;
    }

    // 4. pair
    // 二元组，first second 的具体内容由具体算法或容器决定
    {
        int a1[] = {2, 3, 4, 5};
        int a2[] = {2, 3, 5, 6};
        pair<int *, int *> res = mismatch(a1, a1 + sizeof(a1)/sizeof(*a1), a2);
        cout << "首次出现不匹配的位置: " << (res.first - a1) << endl;
        cout << "对应的数值" << *res.first << " != " << *res.second << endl;
    }

    // ----------- 变异算法

    // 5. copy
    {
        int a[] = {1, 2, 3, 4};
        int b[4];
        vector<int> v;

        // copy(_II __first, _II __last, _OI __result)
        copy(a, a + sizeof(a)/sizeof(*a), b);
        for(auto var : b) {
            cout << var << " ";
        }
        cout << endl;

        copy(a, a + sizeof(a)/sizeof(*a), back_inserter(v));
        for(auto var : v) {
            cout << var << " ";
        }
        cout << endl;

    }

    // 6. swap
    {
        int a = 10;
        int b = 20;
        // swap(_Tp& __a, _Tp& __b)
        swap(a, b);
        cout << a << " " << b << endl;

        // 普通数组需要保证空间大小
        int a1[] = {1, 2, 3, 4};
        int a2[] = {11, 22, 33, 44, 55};

        swap_ranges(a1, a1 + sizeof(a1)/sizeof(*a1), a2);
        copy(a1, a1 + sizeof(a1)/sizeof(*a1), ostream_iterator<typeof(*a1)>(cout, " "));
        cout << endl;
        copy(a2, a2 + sizeof(a2)/sizeof(*a2), ostream_iterator<typeof(*a2)>(cout, " "));
        cout << endl;

        // 容器大小不同也能交换
        vector<int> v1({1, 2, 3, 4, 5});
        vector<int> v2({11, 22, 33});
        swap(v1, v2);
        copy(v1.begin(), v1.end(), ostream_iterator<typeof(v1[0])>(cout, " "));
        cout << endl;
        copy(v2.begin(), v2.end(), ostream_iterator<typeof(v2[0])>(cout, " "));
        cout << endl;
    }

    // 7. transform
    {
        cout << "--- transform" << endl;
        string line;
        vector<string> v;
        vector<string> v2;
        ifstream in("./1.tmp");
        while (!in.eof()) {
            getline(in, line, '\n');
            v.push_back(line);
        }
        in.close();
        // 对 [__first1, __last) 进行 __unary_op ，将__unary_op返回值加入__result
        // transform(_InputIterator __first, _InputIterator __last,
        //    _OutputIterator __result, _UnaryOperation __unary_op)
        transform(v.begin(), v.end(), back_inserter(v2), Encrypt<string>());
        copy(v.begin(), v.end(), ostream_iterator<string>(cout, " "));
        cout << endl;
        copy(v2.begin(), v2.end(), ostream_iterator<string>(cout, " "));
        cout << endl;
    }

    // 8. replace
    {
        cout << "--- replace" << endl;
        array<int, 5> a = {1, 2, 3, 4, 2}; 
        // replace(_ForwardIterator __first, _ForwardIterator __last,
        //  const _Tp& __old_value, const _Tp& __new_value)
        // replace(a.begin(), a.end(), a[1], 10); // 只有第一个2被替换，因为__old_value是引用，当第一个2被替换后，之后__old_value成了10
        replace(a.begin(), a.end(), 2, 10);
        copy(a.begin(), a.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    // 9. generate_n
    {
        cout << "--- generate_n" << endl;
        vector<int> v(8);
        cout << "size" << v.size() << endl;;
        cout << "cap" << v.capacity() << endl;;
        // generate_n(_OutputIterator __first, _Size __n, _Generator __gen)
        // 遍历序列将所有值修改为 __gen的返回值
        generate_n(v.begin(), v.size(), []() -> int {
                int res = rand()%100;
                return res;
                });
        copy(v.begin(), v.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    // 10. unique
    {
        cout << "--- unique" << endl;

        ifstream f("./2.tmp");
        vector<string> v;
        copy(istream_iterator<string>(f), istream_iterator<string>(), back_inserter(v));
        copy(v.begin(), v.end(), ostream_iterator<string>(cout, " "));
        // 由于unique_copy值比较相邻元素，所以需要排序
        sort(v.begin(), v.end());
        cout << endl;

        vector<string> v2;
        // 相邻元素 pred(*i, *(i + 1)) 为真，认为是重复的
        unique_copy(v.begin(), v.end(), back_inserter(v2), [] (const string &s1, const string &s2) -> bool {
                return s1 == s2;
                });
        copy(v2.begin(), v2.end(), ostream_iterator<string>(cout, " "));
        cout << endl;
    }

    // 11. sort
    {
        vector<int> v({2, 1, 4, 3});
        sort(v.begin(), v.end());
        copy(v.begin(), v.end(), ostream_iterator<int>(cout, " "));
        cout << endl;

        list<int> l({2, 1, 4, 3});
        // sort只支持随机迭代器，对于list只能用自己的sort
        l.sort();
        copy(l.begin(), l.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    // 12. binary_search
    {
        list<int> l({2, 2, 1, 1, 4, 4, 4, 3, 3, 3, 6});
        l.sort();
        copy(l.begin(), l.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
        bool is_exist = binary_search(l.begin(), l.end(), 4);
        cout << (is_exist ? "exist" : "not exist") << endl;

        // lower_bound : 在不改变顺序的情况下找到第一个可以插入 val 的位置
        list<int>::iterator it_first = lower_bound(l.begin(), l.end(), 5);
        if (it_first != l.end())
            cout << distance(l.begin(), it_first) << endl;
        else 
            cout << "no" << endl;

        // 在不改变顺序的情况下，查找 val 可能插入的最后一个位置
        list<int>::iterator it_last = upper_bound(l.begin(), l.end(), 5);
        if (it_last != l.end())
            cout << distance(l.begin(), it_last) << endl;
        else 
            cout << "no" << endl;

        pair<list<int>::iterator, list<int>::iterator> p = equal_range(l.begin(), l.end(), 4);
        cout << distance(p.first, p.second) << endl;
        cout << distance(l.begin(), p.first) << endl;
        cout << distance(l.begin(), p.second) << endl;
    }

    return 0;
}




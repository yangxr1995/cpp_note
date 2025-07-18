#include <algorithm>
#include <fstream>
#include <deque>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <vector>
using namespace std;

int main (int argc, char *argv[]) {
    
    // 1. insert_iterator 
    {
        vector<int> v = {1, 2, 3, 4, 5};
        list<int> l = {11, 22, 33};
        // 定义往 v 插入元素的 insert_iterator，插入位置为 v.begin + 3, 即从第4个元素插入
        insert_iterator<vector<int>> insertIterator(v, v.begin() + 3);
        copy(l.begin(), l.end(), insertIterator);
        copy(v.begin(), v.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    // 2. back_insert_iterator front_insert_iterator
    // 需要容器支持 push_front push_back
    {
        deque<int> d = {1, 2, 3, 4, 5};
        list<int> l = {11, 22, 33};
        back_insert_iterator<deque<int>> backInsertIterator(d);
        copy(l.begin(), l.end(), backInsertIterator);
        copy(d.begin(), d.end(), ostream_iterator<int>(cout, " "));
        cout << endl;

        front_insert_iterator<deque<int>> frontInsertIterator(d);
        copy(l.begin(), l.end(), frontInsertIterator);
        copy(d.begin(), d.end(), ostream_iterator<int>(cout, " "));
        cout << endl;
    }

    // 3. reverse_iterator
    //     begin         end
    //     │              │
    //     ▼              ▼
    //     1  2  3  4  5  
    //  ▲              ▲
    //  │              │
    // rend            rbegin  
    {
        vector<int> v = {1, 2, 3, 4, 5};
        vector<int>::reverse_iterator rit(v.end());
        cout << "reverse_iterator : " << *rit << endl;
        cout << "rbegin : " << *v.rbegin() << endl;

        vector<int>::iterator it = find(v.begin(), v.end(), 3);
        vector<int>::reverse_iterator rit2(it);
        cout << "iterator : " << *it << endl;
        cout << "reverse_iterator : " << *rit2 << endl;
    }

    // 4. 
    {
        ostream_iterator<string> out(cout);
        out = "aa";
        out = "bb";
        out = "cc";

        ofstream file;
        file.open("./3.tmp", ios::out);
        vector<string> v = {"aa", "bb", "cc"};
        // 将ofstream 转换为 iterator 
        copy(v.begin(), v.end(), ostream_iterator<string>(file, "\n"));
        file.close();

    }

    return 0;
}

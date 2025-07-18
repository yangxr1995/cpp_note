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

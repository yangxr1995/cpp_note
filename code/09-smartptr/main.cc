#include <iostream>
#include <memory>
using namespace std;

class XData {
public:
    XData() {
        cout << __PRETTY_FUNCTION__ << endl;
    }
    XData(const XData &) {
        cout << __PRETTY_FUNCTION__ << endl;
    }
    XData &operator=(const XData &) {
        cout << __PRETTY_FUNCTION__ << endl;
        return *this;
    }
    ~XData() {
        cout << __PRETTY_FUNCTION__ << endl;
    }

    int a;
    int b;

private:
};

void DelXData(XData *p) {
    cout << __PRETTY_FUNCTION__ << endl;
    delete p;
}

int main (int argc, char *argv[]) {

    // 1. 初步使用
    {
        shared_ptr<int []> sp2(new int [1024]);
        shared_ptr<int> sp1(new int);
        *sp1 = 10;
        sp2[0] = 100;
        auto d1 = sp1.get();
        auto sp3 = make_shared<XData>(); // 堆上XData分配空间
        cout << "sp3.use_count() : " << sp3.use_count() << endl;
    }

    // 2. 复制和复制构造
    {
        auto sp3 = make_shared<XData>();
        auto sp4 = sp3;
        cout << "sp3.use_count() : " << sp3.use_count() << endl;
        cout << "sp4.use_count() : " << sp3.use_count() << endl;
        auto sp5 = make_shared<XData>();
        sp4 = sp5;
        cout << "sp4.use_count() : " << sp3.use_count() << endl;

        {
            auto sp6 = sp3;
            cout << "sp3.use_count() : " << sp3.use_count() << endl;
        }
        sp4 = sp3;
        cout << "} sp3.use_count() : " << sp3.use_count() << endl;
        sp3.reset(); // 释放sp3的所有权，如果此时目标的引用计数为0，则调用释放，否则只是引用计数减一
        cout << "sp3.use_count() : " << sp3.use_count() << endl; // 0
        cout << "sp4.use_count() : " << sp4.use_count() << endl; // 1
    }

    // 3. shared_ptr 定制删除函数
    {
        shared_ptr<XData> sp1(new XData, DelXData);
        shared_ptr<XData> sp2(new XData, [] (XData *p) {
                cout << __PRETTY_FUNCTION__ << endl;
                delete p;
                });
    }

    // NOTE:
    // 4. shared_ptr指向同个对象的不同成员
    {
        shared_ptr<XData> sp1 = make_shared<XData>();
        cout << "sp1 use_count : << " << sp1.use_count() << endl;
        shared_ptr<int> sp2(sp1, &sp1->a);
        cout << "sp1 use_count : << " << sp1.use_count() << endl;
        shared_ptr<int> sp3(sp1, &sp1->b);
        cout << "sp1 use_count : << " << sp1.use_count() << endl;
        cout << "sp3 use_count : << " << sp3.use_count() << endl;
    }

    // 5. shared_ptr循环引用
    {
        class B;
        class A{
            public:
                shared_ptr<B> _p;
                ~A() {
                    _p.reset();
                }
        };
        class B {
            public:
                shared_ptr<A> _p;
                ~B() {
                    _p.reset();
                }
        };

        shared_ptr<A> a = make_shared<A>();
        shared_ptr<B> b = make_shared<B>();

        a->_p = b;
        b->_p = a;
        cout << "a use_count : " << a.use_count() << endl;
        cout << "b use_count : " << b.use_count() << endl;
        // NOTE:
        // 由于循环引用，a,b 释放所有权后，没有调用对象的析构
    }

    // weak_ptr
    //  use_count
    //    返回被管理对象的shared_ptr的对象数量
    //  lock
    //    创建被管理对象的shared_ptr
    // weak_ptr 不会让shared_ptr的use_count增加。从而解决了循环引用的问题
    // 当需要使用对象时，使用lock获得shared_ptr的复制，此时会让引用计数加一
    {
        class A;
        class B {
            public:
                int b = 10;
                weak_ptr<A> _p;
                ~B() {
                    cout << __PRETTY_FUNCTION__ << endl;
                }
        };
        class A{
            public:
                weak_ptr<B> _p;
                void Do() {
                    cout << "use_count : " << _p.use_count() << endl;
                    shared_ptr<B> p = _p.lock();
                    if (p != nullptr) {
                        cout << "use_count : " << _p.use_count() << endl;
                        cout << "b : " << p->b << endl;
                    }
                }
                ~A() {
                    cout << __PRETTY_FUNCTION__ << endl;
                }
        };
        shared_ptr<A> a = make_shared<A>();
        shared_ptr<B> b = make_shared<B>();

        a->_p = b;
        b->_p = a; // NOTE : weak_ptr 不会导致引用计数增加
        cout << "a use_count : " << a.use_count() << endl;
        cout << "b use_count : " << b.use_count() << endl;
        a->Do();
    }




    
    return 0;
}

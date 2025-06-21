# 智能指针的介绍
智能指针的作用是自动回收资源。
原理是利用栈回退时自动调用对象的析构函数。
智能指针对象本身必须是栈变量，其内部为被管理的业务对象的指针，当智能指针析构时，会调用业务对象的析构(引用计数减一)。

# 不带引用参数的智能指针
## 实现原理
```cc
template <typename T>
class Sptr {
    public:
        Sptr<T>(T *p)
        : _p(p) {
        }
        ~Sptr<T> () {
            delete _p;
        }
        T & operator*() {
            return *_p;
        }
        T *operator->() {
            return _p;
        }
    private:
        T *_p;
};

Sptr<Test> p1(new Test());
Sptr<const Test> p2 = new const Test(10);
p2->show();
(*p2).show();
```
       ┌─────────────────────┐         ┌────────────────────┐
       │智能指针对象(栈空间) │         │ 被管理对象(堆空间) │
       │                 _p  ├────────►│                    │
       └─────────────────────┘         └────────────────────┘

## c++11提供的不带引用计数的智能指针
不带引用计数的智能指针用于对象只由一个指针持有的情况。
无法用于多线程。

### auto_ptr
```cc
auto_ptr<Test> p1(new Test());
Sptr<const Test> p2 = new const Test(10);
p2->show();
(*p2).show();
```

auto_ptr已经被废弃，原因是赋值和拷贝构造会导致原对象的内部指针置为nullptr，容易误导开发者
```cc
auto_ptr<Test> p1(new Test());
auto_ptr<Test> p2 = p1; // p1 内部的指针置为nullptr
p2->show();
p1->show(); // error
```
### scoped_ptr
scoped_ptr不是c++11的标准，而是boost的实现，特点是严格了拷贝构造和赋值操作。

### unique_ptr
unique_ptr只提供了右值引用的赋值和拷贝构造，确保开发者显示知道对象所有权的转移。
```cc
unique_ptr<Test> p1(new Test());
unique_ptr<Test> p2 = std::move(p1);
// unique_ptr<Test> p2 = p1; // err
```

```cc
unique_ptr<Data> p1 = new Data();
unique_ptr<Data> p2 = new Data();
p1 = move(p2);
// 重置空间
p1.reset(new Data());

//主动释放空间
p1 = nullptr;
unique_ptr<int> p2(new int);
// 释放所有权，返回原始指针
auto p3 = p2.release();
delete p3;

// 自定义删除方法
#include <iostream>
#include <memory>
using namespace std;

struct XPacket {
    void *data;
    int size;
};

class PacketDelete {
    public:
        void close() {
            cout << __PRETTY_FUNCTION__ << endl;
        }
        void operator()(struct XPacket *p) const {
            cout << __PRETTY_FUNCTION__ << endl;
            delete p;
        }
};

int main (int argc, char *argv[]) {

    {
        unique_ptr<XPacket, PacketDelete> p1(new XPacket);

        // 手动分步调用释放
        unique_ptr<XPacket, PacketDelete> p2(new XPacket);
        //     get_deleter() 获得函数对象
        // 智能指针.xxx  : 访问智能指针内部属性
        // 智能指针->xxx : 访问智能指针指向的对象属性
        p2.get_deleter().close();
        p2.get_deleter()(p2.get());
        p2.release();
    }
    
    return 0;
}
```

# 带引用计数的智能指针
带引用计数的智能指针的优点是，多个模块可以管理同个资源。

## 实现原理
原理是使用引用计数对象，当添加智能指针指向资源时，资源相关的引用计数加一，当智能指针析构时，相关的引用计数减一，当引用计数为0时，释放资源。

```cc
template <typename T>
class Refcnt {
    public:
        Refcnt(T *p)
        : _p(p), _cnt(1) {
        }
        int get() {
            return ++_cnt;
        }
        int put() {
            if (_cnt == 0)
                throw "_cnt == 0";
            return --_cnt;
        }
        ~Refcnt() {
            delete _p;
        }
    private:
        T *_p;
        int _cnt;
};

template <typename T>
class Sptr {
    public:
        Sptr<T>(T *p)
            // 被资源加入智能指针模块时为其创建引用计数对象
        : _p(p), _refcnt(new Refcnt<T>(p)) {
        }
        ~Sptr<T> () {
            if (_refcnt->put() == 0)
                delete _refcnt;
        }
        Sptr<T> (const Sptr<T> &sp) 
        : _p(sp._p), _refcnt(sp._refcnt) {
            _refcnt->get();
        }
        T & operator*() {
            return *_p;
        }
        T *operator->() {
            return _p;
        }
    private:
        T *_p;
        Refcnt<T> *_refcnt;
};
```

## c++11提供的带引用计数的智能指针
### shared_ptr weak_ptr
#### shared_ptr
- 共享所有权的智能指针，多个shared_ptr指向同个对象
- 可复制构造
- 可复制赋值
- 数据访问非线程安全
- shared_ptr的控制块是线程安全

实现原理
1. 数据指针，可通过get()返回
2. 控制块指针（动态分配，被管理对象，删除和分配器，引用数量）


         ┌───────────────────┐
         │shared_ptr         │            shared_ptr<int> sp1(new int)             ┌───────────────────────┐
         ├───────────────────┴──┐       ┌───────────────────────┐         ┌───────►│ 被管理的数据的内存空间│◄────┐
         │element_type *_Ptr    │       │ element_type *_Ptr ───┼─────────┘        └───────────────────────┘     │
         │  数据指针            │       │    数据指针           │                                                │
         │  非线程安全          │       │ _Ref_count_base *_Rep─┼────────►引用计数=1                             │
         ├──────────────────────┤       │    控制块指针         │            │     ┌───────────────────────┐     │
         │_Ref_count_base *_Rep │       └───────────────────────┘            └───► │ 控制块(引用计数)      │     │
         │  控制块              │                                                  └───────────────────────┘     │
         │  引用计数            │        shared_ptr<int> sp2 = sp1                              ▲                │
         │  删除和分配器        │       ┌───────────────────────┐                               │                │
         │  线程安全            │       │element_type *_Ptr─────┼───────────────────────────────┼────────────────┘
         └──────────────────────┘       │   数据指针            │                               │
                                        │_Ref_count_base *_Rep──┼───────────────► 引用计数+1=2──┘
                                        │   控制块指针          │    
                                        └───────────────────────┘


```cc
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
```


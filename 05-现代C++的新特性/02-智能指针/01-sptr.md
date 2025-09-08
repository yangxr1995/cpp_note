# 智能指针的介绍
智能指针的作用是自动回收资源。
原理是利用栈回退时自动调用对象的析构函数。
智能指针对象本身必须是栈变量，其内部为被管理的业务对象的指针，当智能指针析构时，会调用业务对象的析构(引用计数减一)。

# 基本原理
## 不带引用参数的智能指针
### 实现原理
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

## 带引用计数的智能指针
带引用计数的智能指针的优点是，多个模块可以管理同个资源。

### 实现原理
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


# STL提供的智能指针

C++有4个智能指针: auto_ptr, unique_ptr, shared_ptr, weak_ptr, 其中 auto_ptr被弃用。
几个智能指针的特点:
- unique_ptr: 独占对象所有权，由于没有引用计数，因此性能较好
- shared_ptr: 共享对象所有权，但性能差
- weak_ptr: 配合 shared_ptr 解决循环引用的问题

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


### shared_ptr 
#### shared_ptr 的工作原理
        std::shared_ptr<T> buf                      目标数据                           std::shared_ptr<T> buf 
        ┌──────────────────────┐                   ┌────────────┐                     ┌───────────────────────┐
        │Ptr to T              │─────────────────► │T Object    │ ◄───────────────────│Ptr to T               │
        │Ptr to Control Block  │────────┐          └────────────┘             ┌───────│Ptr to Control Block   │
        └──────────────────────┘        │                                     │       └───────────────────────┘
                                        │           Control Block(线程安全)   │
                                        │          ┌────────────────┐         │
                                        └────────► │Reference Count │◄────────┘
                                                   │Weak Count      │
                                                   │Other Data( eg: │
                                                   │custom deleter, │
                                                   │allocator, ..)  │
                                                   └────────────────┘
```cc
{
    shared_ptr<Buffer> buf(new Buffer("auto free memory"));
    shared_ptr<Buffer> buf2 = buf;
}
```


#### 初始化
初始化智能指针有三种方法:
1. 原始指针做构造参数参数
2. 智能指针的拷贝构造
3. make_shared

```cc
shared_ptr<int> p1(new int(1));
shared_ptr<int> p2 = p1;
shared_ptr<int> p3;
p3.reset(new int(2));
```

```cc
auto sp1 = make_shared<int>(100);
shared_ptr<int> sp2 = make_shared<int>(200);
// 相当于
shared_ptr<int> sp1(new int(100));
shared_ptr<int> sp2(new int(200));
```

注意不能这样:
```cc
shared_ptr<int> sp = new int(100);
```

#### 获取原始指针
若需要获得原始指针，使用get()
```cc
shared_ptr<int> sp = make_shared<int>(100);
int *p = sp.get();
```

#### 自定义删除方法
```cc
shared_ptr<XData> sp1(new XData, DelXData);
shared_ptr<XData> sp2(new XData, [] (XData *p) {
        cout << __PRETTY_FUNCTION__ << endl;
        delete p;
    });
```

#### 指向同个对象的不同成员
```cc
shared_ptr<XData> sp1 = make_shared<XData>();
cout << "sp1 use_count : << " << sp1.use_count() << endl; // 1
shared_ptr<int> sp2(sp1, &sp1->a);
cout << "sp1 use_count : << " << sp1.use_count() << endl; // 2
shared_ptr<int> sp3(sp1, &sp1->b);
cout << "sp1 use_count : << " << sp1.use_count() << endl; // 3
cout << "sp3 use_count : << " << sp3.use_count() << endl; // 3
```

#### weak_ptr解决循环引用
```cc
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
```

#### 使用 shared_ptr 需要注意的问题
##### 不要使用原始指针初始化多个 shared_ptr
```cc
int *p = new int;
shared_ptr<int> sp1(p);
shared_ptr<int> sp2(p); // 创建新的 Control Block
```

##### 不要在函数实参中创建 shared_ptr
```cc
function(shared_ptr<int>(new int), g());
```
应该改为
```cc
auto sp = make_shared<int>();
function(sp, g());
```

##### 不要通过 shared_from_this() 返回this指针
```cc
class A {
    public:
    shared_ptr<A> GetSelf() {
        return shared_ptr<A>(this);
    }
};

int main() {
    shared_ptr<A> sp1(new A);
    shared_ptr<A> sp2 = sp1->GetSelf();
}
```

### weak_ptr
`weak_ptr<T>` 作为 `shared_ptr<T>` 的观察者，不修改 `shared_ptr<T>` 的count。以解决 shared_ptr 的循环引用问题
#### 基本用法
1. 通过 use_count 获取当前观察资源的引用计数
```cc
shared_ptr<int> sp(new int(1));
weak_ptr<int> wp(sp);
cout << wp.use_count() << endl; // 1
```
2. 通过 expired 判断锁观察的资源是否已经释放。
```cc
shared_ptr<int> sp(new int (1));
weak_ptr<int> wp(sp);
if (wp.expired())
    cout << "weak_ptr 无效，资源已经释放"
```
3. 通过lock方法获取监视的shared_ptr
```cc
weak_ptr<int> wp;
void f() {
    auto sp = wp.lock(); // shared_ptr use_count ++
    if (wp.expired()) {
        cout << "wp 无效，资源已经释放" << endl;
    }
    else {
        cout << "wp 有效，val = " << *sp << endl;
    }
}
```


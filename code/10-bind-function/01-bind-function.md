# 模板的特例化和实参推演
## 特例化

特例化分为完全特例化和部分特例化，
完全特例化是在定义类时用确定类型对类进行特例化如
```cc
template <>
class Test<const char *> 
```

部分特例化是在定义类时用类型参数对类进行特例化如
```cc
template <typename T>
class Test<T *>

```

```cc
#include <iostream>
using namespace std;

// 通用模板
template <typename T>
class Test {
    public:
        void func(T a) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
};

// 通用模板Test的完全特例化
// 注意特例化版本不能单独存在，必须要有通用模板才能定义相关特例化版本
template <>
class Test<const char *> {
    public:
        void func(const char *a) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
};

// 通用模板Test的指针类型的部分特例化
template <typename T>
class Test<T *> {
    public:
        void func(T *a) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
};

// 通用模板Test的函数指针类型的部分特例化
template <typename R, typename C, typename A>
class Test<R (C::*)(A)> {
    public:
        void func(R (C::*p)(A)) {
            cout << __PRETTY_FUNCTION__ << endl;
            C c;
            A a;
            (c.*p)(a);
        }
};

// 函数类型可以定义，只有函数指针类型才能使用，所以用函数类型定义函数指针类型
template <typename R, typename A1, typename A2>
class Test<R (A1, A2)> {
    public:
        void func(R (*p)(A1, A2)) {
            cout << __PRETTY_FUNCTION__ << endl;
            A1 a1;
            A2 a2;
            p(a1, a2);
        }
};

int main (int argc, char *argv[]) {

    {
        Test<int> t1;
        t1.func(1);
        Test<const char *> t2;
        t2.func("aa");

        Test<int *> t3;
        t3.func(new int);

        void (Test<int>::*aa) (int) = &Test<int>::func;
        (t1.*aa)(1);

        Test<void (Test<int>::*)(int)> t4;
        t4.func(&Test<int>::func);

    }

    return 0;
}
```

## 实参推演
编译器会根据模板函数的传递实参，进行模板类型推演

```cc
#include <iostream>
using namespace std;

template <typename T>
void func1(T a) {
    cout << __PRETTY_FUNCTION__ << endl;
    cout << typeid(T).name() << endl;
}

template <typename T>
void func1(T *a) {
    cout << __PRETTY_FUNCTION__ << endl;
    cout << typeid(T).name() << endl;
}

template <typename R, typename A1, typename A2>
void func1(R (*a)(A1, A2)) {
    cout << __PRETTY_FUNCTION__ << endl;
    cout << typeid(R).name() << endl;
    cout << typeid(A1).name() << endl;
    cout << typeid(A2).name() << endl;
}

template <typename R, typename C, typename A1, typename A2>
void func1(R (C::*a)(A1, A2)) {
    cout << __PRETTY_FUNCTION__ << endl;
    cout << typeid(R).name() << endl;
    cout << typeid(C).name() << endl;
    cout << typeid(A1).name() << endl;
    cout << typeid(A2).name() << endl;
}

class Test {
    public:
        void add(int a, int b) { return ;}
};

void add(int a, int b) { return ;}

int main (int argc, char *argv[]) {

    func1(1);
    func1(new int);
    func1(add);
    func1(&Test::add);
    
    return 0;
}
```

# functional
## 介绍
functional的功能是作为中间层，可以将函数，lambada转换为function， 
function是函数对象，能给泛型算法使用。
```cc
#include <iostream>
#include <functional>
using namespace std;

void fun1(int a) {
    cout << __PRETTY_FUNCTION__ << endl;
}

void fun2(int a, int b) {
    cout << __PRETTY_FUNCTION__ << endl;
}

int main (int argc, char *argv[]) {
    function<void (int)> f1 = fun1;
    f1(1);
    function<void (int, int)> f2 = fun2;
    f2(1, 2);

    return 0;
}
```


## 原理
functional就是用类对函数进行封装，将其转换为函数对象，原理是利用可变参函数模板。
```cc
#include <iostream>
using namespace std;

template <typename R, typename... A>
class myfunctional {
    using PFUNC = R(*)(A...);

    public:
        myfunctional(PFUNC p)
        : _p(p) {
        }
        R operator()(A... a) {
            return _p(a...);
        }
    private:
        PFUNC _p;
};

void fun1(int a) {
    cout << __PRETTY_FUNCTION__ << endl;
}

void fun2(int a, int b) {
    cout << __PRETTY_FUNCTION__ << endl;
}

int main (int argc, char *argv[]) {
    myfunctional f1 = fun1;
    f1(1);
    myfunctional f2 = fun2;
    f2(1, 2);
    
    return 0;
}
```
# 绑定器
## bind1st bind2nd
- bind1st : 封装二元函数对象，将第一个形参变量绑定为确定的值
- bind2nd : 封装二元函数对象，将第二个形参变量绑定为确定的值

```cc
#include <cstdlib>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
using namespace std;

int main (int argc, char *argv[]) {

    vector<int> v;
    for (int i = 0; i < 10; i++) {
        v.push_back(rand()%100 + 1);
    }

    // greater<int>() 返回真，则插入，说明从大到小排序
    sort(v.begin(), v.end(), greater<int>());

    for (int i : v) {
        cout << i << " ";
    }
    cout << endl;

    // find_if 只接受一元函数对象，用bind1st将greater转换为一元
    // 当 70 > *it 时，返回it
    auto it = find_if(v.begin(), v.end(), bind1st(greater<int>(), 70));
    if (it != v.end()) {
        cout << "it : " << *it << endl;
        v.insert(it, 70);
    }

    for (int i : v) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
```
## bind1st 的原理
```cc
#include <cstdlib>
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>
using namespace std;

// OP : 函数对象类，OP::result_type 函数对象的属性，如果传入函数需要用functional转换为函数对象
// A : 第一个参数类型
template <typename OP, typename A>
class mybind1st {
    public:
        // const OP & :  可以接受右值(临时量)
        mybind1st(const OP &op, const A &a)
        : _op(op), _a1(a) {}
        typename OP::result_type operator()(A a2) const {
            return _op(_a1, a2);
        }
    private:
        OP _op;
        A _a1;
};

int main (int argc, char *argv[]) {

    vector<int> v;
    for (int i = 0; i < 10; i++) {
        v.push_back(rand()%100 + 1);
    }

    // greater<int>() 返回真，则插入，说明从大到小排序
    sort(v.begin(), v.end(), greater<int>());

    for (int i : v) {
        cout << i << " ";
    }
    cout << endl;

    // find_if 只接受一元函数对象，用bind1st将greater转换为一元
    // 当 70 > *it 时，返回it
    auto it = find_if(v.begin(), v.end(), mybind1st(greater<int>(), 70));
    if (it != v.end()) {
        cout << "it : " << *it << endl;
        v.insert(it, 70);
    }

    for (int i : v) {
        cout << i << " ";
    }
    cout << endl;

    return 0;
}
```

## bind
### 功能介绍
bind可以将函数/函数对象绑定参数并返回函数对象。
他和functional功能类似，但是functional只能将函数转换为函数对象，bind还可以对参数进行绑定
bind1st只能对函数对象进行参数绑定，并且只支持二元函数对象转换为一元函数对象。bind可以绑定最多20个参数的函数对象。

```cc
#include <cstdlib>
#include <iostream>
#include <functional>
using namespace std;

void hello()
{
    cout << __PRETTY_FUNCTION__ << endl;
}

int add(int a, int b)
{
    return a+b;
}

class Test {
    public:
        Test() {}
        void func(int a) {
            cout << __PRETTY_FUNCTION__ << endl;
        }
};

int main (int argc, char *argv[]) {

    function<void ()> a = bind(hello);
    a();
    function<int ()> b = bind(add, 1, 2);
    cout << b() << endl;

    // 和占位符使用
    function<int (int, int)> c = bind(add, placeholders::_1, placeholders::_2);
    cout << c(1, 2) << endl;
    function<int (int)> d = bind(add, 1, placeholders::_1);
    cout << d(1) << endl;

    bind(&Test::func, placeholders::_1, placeholders::_2)(Test(), 1);

    return 0;
}
```

# lambada
lambada的作用是快速定义函数对象

语义如下
```txt
[捕获外部变量列表] (形参列表) mutable -> 返回值 { 工作代码 }

捕获外部变量列表可以为：
[] : 不捕获
[=] : 外部变量传值捕获全部
[&] : 外部变量传地址捕获全部
[this] : 捕获外部的this指针
[=, &a] 
[a, b]
```

## lambada的原理
```cc
#include <cstdlib>
#include <iostream>
using namespace std;

class Lambada1 {
    public:
        Lambada1() {}
        int operator()(int a, int b) {
            return a + b;
        }
};

class Lambada2 {
    public:
        Lambada2(int *pn1, int *pn2)
        : _pn1(pn1), _pn2(pn2) {}
        void operator() () {
            int tmp = *_pn1;
            *_pn1 = *_pn2;
            *_pn2 = tmp;
        }
    private:
        int *_pn1;
        int *_pn2;
};
 
int main (int argc, char *argv[]) {

    auto a = [] (int a, int b) -> int {
        return a + b;
    };
    auto aa = Lambada1();
    cout << aa(1, 2) << endl;
    cout << a(1, 2) << endl;

    int n1 = 1, n2 = 2;
    auto b = [&n1, &n2] () -> void {
        int tmp;
        tmp = n1;
        n1 = n2;
        n2 = tmp;
    };
    b();
    cout << n1 << " " << n2 << endl;
    auto bb = Lambada2(&n1, &n2);
    bb();
    cout << n1 << " " << n2 << endl;
 
    return 0;
}
```

## lambada的应用
```cc
unique_ptr<FILE, function<void (FILE*)>> ptr(fopen("aa", "w"), [](FILE *fp) {fclose(fp);});
```

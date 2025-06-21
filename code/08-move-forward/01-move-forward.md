
# 左值，右值
## 左值（Lvalue）
左值指具有明确内存地址的表达式，通常可以出现在赋值运算符=的左侧。其名称中的"L"最初表示"Left"，但更准确的理解是"Location"（可寻址）。

2.核心特性
可寻址：可通过&运算符获取地址（如&a）。
持久性：生命周期较长，如变量、对象或数组元素。
可修改性：非const左值可被赋值（如a = 10）。

## 右值（Rvalue）
1.定义
右值指临时数据或无法寻址的表达式，通常出现在赋值运算符右侧。其名称中的"R"可理解为"Read"（仅可读）或"Register"（可能存储在寄存器中）。

2.核心特性
不可寻址：无法通过&获取地址（如&(a + b)会报错）。
临时性：生命周期短暂，如字面量或表达式结果。
不可修改：不能直接赋值（如10 = a无效）。

3.常见示例
匿名变量


# 匿名变量
匿名变量没有符号，所以生命周期为当前语句，当前语句结束，匿名变量析构
```cc
Test t1;
t1 = Test(1); // 1. 匿名变量构造 Test(int)
              // 2. operator=(const Test &)
              // 3. 匿名变量析构 ~Test()
```
编译器会对匿名变量优化
```cc
Test t1 = Test(1); // 优化为 Test t1(1);
```
引用可以为匿名变量添加名称，让匿名变量变成普通变量，生命周期延长
```cc
Test &t = Test(1); // 相当于 Test t(1);
```
指针无法为匿名变量延长生命周期，所以容易导致野指针
```cc
Test *pt = &Test(1); // 语句结束后匿名变量立即析构，导致指针悬挂
```
## 匿名变量和函数调用
母函数和子函数的栈帧不同，导致双方的局部变量不能互相访问，但是当母函数有需要读取子函数返回值的需求。编译器通过在母函数栈帧上分配匿名变量，并传递匿名变量的地址给子函数，让子函数直接写匿名变量以返回值给母函数。
```cc
main() {
    Test t1;
   10c2c:	sub	r3, fp, #32
   10c30:	mov	r1, #0
   10c34:	mov	r0, r3
   10c38:	bl	10fc8 <Test::Test(int)>
    Test t2;
   10c3c:	sub	r3, fp, #28
   10c40:	mov	r1, #0
   10c44:	mov	r0, r3
   10c48:	bl	10fc8 <Test::Test(int)>
     get_obj(t1);
   10c4c:	sub	r2, fp, #32 ; 获得实参t1地址
   10c50:	sub	r3, fp, #24 ; 获得形参a的地址
   10c54:	mov	r1, r2   ; 传递实参t1的地址
   10c58:	mov	r0, r3   ; 传递形参a的地址为this
   10c5c:	bl	11014 <Test::Test(Test const&)> ；构造形参a
   10c60:	sub	r3, fp, #20 ; 分配匿名变量的内存
   10c64:	sub	r2, fp, #24 ; 获得形参a的地址
   10c68:	mov	r1, r2      ; 传递参数a的地址
   10c6c:	mov	r0, r3      ; 传递匿名变量的地址
   10c70:	bl	10bd4 <get_obj(Test)>
   10c74:	sub	r3, fp, #20    ; 获得匿名变量地址
   10c78:	mov	r0, r3         ; 匿名变量地址做this
   10c7c:	bl	11060 <Test::~Test()> ; 匿名变量析构
   10c80:	sub	r3, fp, #24
   10c84:	mov	r0, r3
   10c88:	bl	11060 <Test::~Test()> ; 形参a的析构
    return 0;
}

Test get_obj(Test a)
{
   10bd4:	push	{fp, lr}
   10bd8:	add	fp, sp, #4
   10bdc:	sub	sp, sp, #16
   10be0:	str	r0, [fp, #-16] ; 获得匿名变量地址
   10be4:	str	r1, [fp, #-20] ; 获得形参a的地址
    int val = a._a;
   10be8:	ldr	r3, [fp, #-20]	; 从形参a获得val
   10bec:	ldr	r3, [r3]
   10bf0:	str	r3, [fp, #-8] ; 
    Test tmp(val);
   10bf4:	ldr	r1, [fp, #-8] ; 传参a
   10bf8:	ldr	r0, [fp, #-16] ; 匿名变量做this指针
   10bfc:	bl	10fc8 <Test::Test(int)>
    return tmp;
   10c00:	nop			@ (mov r0, r0)
}
   10c04:	ldr	r0, [fp, #-16] ; 返回匿名变量
   10c08:	sub	sp, fp, #4
   10c0c:	pop	{fp, pc}
```
当编译发现函数返回类对象时，就会在母函数开辟匿名变量，并将匿名变量的地址作为参数1传递，匿名变量作为返回值。
```cc
Test get_obj(Test a);
// 会被修改为
Test get_obj(Test *panonymous, Test a);
```
如此可以优化函数返回变量时，额外的对象构造、析构、赋值

# 右值引用
由于匿名对象有读后即删的特点，这会导致cpp代码大量的冗余，比如如下代码
```cc
String get_string(String &s)
{
    const char *p = s.cstr();
    return String(p); // 1. String(const char *)
}
s2 = get_string(s1); // 2. operator=(const String &)
```
当使用匿名变量对s2进行赋值时，需要先删除s2原有的字符串空间，并创建新的空间然后拷贝匿名变量的字符串，最后匿名变量析构时会删除匿名变量的字符串空间。
显然匿名变量的内部堆空间可以通过转交给s2，以节省上诉的冗余开销。
但问题是赋值函数无法区分参数为普通变量和匿名变量。
所以c++11提供了右值引用，以实现区分普通变量和匿名变量，因为匿名变量为右值，所以匿名变量的赋值方法的参数为右值引用，普通变量的赋值方法的参数为左值引用。
```cc
String operator=(String &&s);
String operator=(const String &s);
```
所以赋值方法可以实现为
```cc
String & operator=(String &&s) {
    delete [] _s;
    _s = s._s;
    s._s = nullptr;
    return *this;
}
String & operator=(const String &s) {
    delete [] _s;
    _s = new char [s.len() + 1]{0};
    strcpy(_s, s._s);
    return *this;
}
```
同样拷贝构造函数可以改为
```cc
String(String &&s) {
    _s = s._s;
    s._s = nullptr;
}
String(const String &s) {
    _s = new char [strlen(s._s) + 1] {0};
    strcpy(_s, s._s);
}
```
## 右值引用和常引用
右值引用和常引用的汇编代码是一样的，只是编译角度有差别
```cc
const String &s = String("111");
String &&s = String("1111");
```
常引用的目标只读，右值引用的目标可修改，
常引用做函数参数时，无法区分左值做参数的调用和右值做参数的调用。
但右值引用可以区分。
右值引用只能引用右值，不能引用左值，但常用引用都可以。

# 转移语义和完美转发
默认情况，普通对象是左值，只有匿名对象是右值，有时我们知道某普通对象不再使用，则希望将他的堆空间转移给新的对象，但普通对象本身是左值，这时就需要用转移语义.
```cc
String s1("1234");
String s2 = std::move(s1); // String(String &&)
// 以后s1不再使用，随栈析构
```
move的原理是强制将对象由其左值类型转换为该对象的右值类型。
## std::move —— 容器模板和右值引用
转移语义大量用在容器模板，
比如String定义了右值引用的构造，但Vector中没有使用转移语义，会导致实际调用为String的左值引用拷贝构造.
```cc
template <typename T>
struct Allocator {
    void construct(T *p, const T &val) {
        new (p) T(val); // val为左值，调用左值的拷贝构造
    }
};

template <typename T, typename Alloca = Allocator<T>>
class Vector {
        void push_back(const T &a) { // 常引用可以引用右值，但常引用本身为左值
            if (full())
                expand();
            _alloca.construct(_last, a); // 所以这里调用左值的 construct
            ++_last;
        };
        }

Vector<String> v1;
v1.push_back(String("111")); // String(const String &)
v1.push_back(String("222")); // String(const String &)
```
使用std::move将左值转为右值。
```cc
template <typename T>
struct Allocator {
    void construct(T *p, const T &val) {
        new (p) T(val); // val为左值，调用左值的拷贝构造
    }
    void construct(T *p, T &&val) {
        new (p) T(std::move(val)); // val为左值，但std::move(val)为右值，调用右值的拷贝构造
    }
};

template <typename T, typename Alloca = Allocator<T>>
class Vector {
        void push_back(const T &a) { // 常引用可以引用右值，但常引用本身为左值
            if (full())
                expand();
            _alloca.construct(_last, a); // 所以这里调用左值的 construct
            ++_last;
        };
        void push_back(T &&a) { // 常引用可以引用右值，但常引用本身为左值
            if (full())
                expand();
            _alloca.construct(_last, std::move(a)); // std::move(a)为右值
            ++_last;
        };
};

Vector<String> v1;
v1.push_back(String("111")); // String(String &&)
v1.push_back(String("222")); // String(String &&)
```
## std::forward 简化代码
std::move虽然能实现调用右值引用的方法，但是模板需要同时实现左值和右值两个方法，且代码高度重复，
使用std::forward可以简化代码。
std::forward的作用是，被转发的引用指向右值，则将引用本身转换为右值，若引用指向左值，则将引用本身转换为左值。
```cc
template <typename T>
struct Allocator {
    template<typename Ty>
    void construct(T *p, Ty &&val) {
        new (p) T(std::forward<Ty>(val));
    }
};

template <typename T, typename Alloca = Allocator<T>>
class Vector {
    template<typename Ty>
    void push_back(Ty &&a) { // 引用折叠， Ty 为String && 时，定义 String && &&a -> String && a
        if (full())          //            Ty 为String &时，  定义 String & &&a  -> String &  a
            expand();
        _alloca.construct(_last, std::forward<Ty>(a));
        ++_last;
    }
};

Vector<String> v1;
v1.push_back(String("111")); // String(String &&)
v1.push_back(String("222")); // String(String &&)
```
注意forward需要显示指定模板参数
```cc
std::forward(a); // err
std::forward<Ty>(a); // ok
```



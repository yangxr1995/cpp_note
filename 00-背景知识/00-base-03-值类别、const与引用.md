[[toc]]

# 值类别、const 与引用

## 1. 值类别

C++ 使用以下术语描述表达式：

~~~text
表达式
├─ glvalue：能定位到某个对象或函数
│  ├─ lvalue：通常表示持久对象或函数
│  └─ xvalue：对象资源可被复用的将亡值
└─ rvalue
   ├─ xvalue
   └─ prvalue：纯右值，常用于初始化或计算结果
~~~

~~~cpp
int x = 1;
int& lref = x;                 // x 是 lvalue
int&& rref = 1;                // 1 是 prvalue
int&& moved = std::move(x);    // std::move(x) 是 xvalue
~~~

“左值在等号左边、右值在等号右边”只是记忆法，不是定义。命名变量即使类型是 T&&，其表达式仍是 lvalue。

std::move 不负责移动资源，只把表达式转换成 xvalue，使移动构造或移动赋值有机会被选中。

## 2. 临时对象和生命周期

~~~cpp
const std::string& a = std::string("tmp"); // 延长到 a 的作用域结束
auto&& b = std::string("tmp");             // 同样延长
~~~

函数参数中的临时对象通常只活到本次调用的完整表达式结束：

~~~cpp
void consume(const std::string&);
consume(std::string("tmp")); // 函数返回后不能保存该引用
~~~

不要返回局部对象或临时对象的引用：

~~~cpp
const std::string& bad() {
    return std::string("temporary"); // 悬空引用
}
~~~

## 3. const 修饰指针

~~~cpp
const int* p1;                  // 可改 p1，不可通过 p1 改 int
int* const p2 = nullptr;        // 不可改 p2，可改 *p2（若非空）
const int* const p3 = nullptr;  // 指针和值的视角都只读
~~~

顶层 const 修饰指针本身，底层 const 修饰指向的对象：

~~~cpp
int n = 1;
int* const& ref = &n;       // 对 const 指针的引用
*ref = 2;                   // 合法
// ref = nullptr;           // 错误

const int* const& view = &n;
// *view = 3;                // 错误
~~~

虽然 int* 可以转换成 const int*，但不能把这种转换偷偷放进对指针本身的非常量引用中，否则会破坏类型安全。

## 4. const 对象和常量表达式

~~~cpp
const int a = 10;
static_assert(a == 10);
int array[a]; // a 是整型常量表达式，可用作 C++ 固定数组边界
~~~

这不意味着 a 一定没有存储。编译器可能传播它的值，也可能因为取地址、跨翻译单元或 ABI 需要而分配存储。

修改原本就是 const 的对象属于未定义行为：

~~~cpp
const int a = 10;
int* p = const_cast<int*>(&a);
*p = 20; // 未定义行为
~~~

若原对象本来不是 const，只是通过 const 视角访问，则去掉视角上的 const 可以合法：

~~~cpp
int x = 10;
const int* view = &x;
*const_cast<int*>(view) = 20; // 合法，但通常没有必要
~~~

const 与 constexpr 不等价；C 与 C++ 对常量表达式、默认链接属性和数组边界的规则也不完全相同。

## 5. 引用的语义

引用的语言语义是已存在对象的别名：

~~~cpp
int x = 10;
int& r = x;
r = 20; // 修改 x
~~~

常见 ABI 会把引用参数实现为地址，但标准不保证引用一定占一个指针大小、一定在内存中有独立槽位，或一定可以被调试器观察到。

引用不能重新绑定；需要可为空、可改指向的接口应使用指针或其他明确抽象。模板中的 T&& 还可能是转发引用，需要配合 std::forward<T> 完美转发。

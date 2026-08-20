# 基础

## const如何使用
### const修饰基础类型
const 修饰的变量必须初始化
```cc
const int a = 10;
const int b;      // ERROR
```
### const修饰指针
需要明确const修饰的是指针常量还是指针指向的变量
```cc
int a;
// 常量指针：(*p) 为 const
const int *p1 = &a;
int const *p1 = &a;

// 指针常量: p 为const
int * const p2 = &a;
```

## C++中const的实现原理
### C语言中的const
- 本质是只读变量
  - C语言中const修饰的变量称为“常变量”，其值在运行时确定。虽然语法上禁止直接修改，但可以通过指针间接修改其内存中的值。
- 编译处理方式
  - C的const变量在汇编层面仍被视为普通变量，编译器不会进行常量替换优化。即使声明为const，其地址仍可被获取，且允许通过指针修改。
- 应用限制
  - 不能用于定义数组长度（如`int arr[a]`会报错）。
  - 不能作为switch语句的常量表达式。

### C++中的const
C++的const行为取决于初始化方式，分为两种情况：

- 1. 用常量初始化时（编译时常量）
行为类似宏替换, 是真正的常量.
无法通过非const指针直接指向const变量（需强制类型转换），且修改临时内存不影响符号表中的常量值。

示例：
```c
const int a = 10; // 编译时确定了常量的值
int *p = (int *)&a;
*p = 30;  // p的确指向了符号a对应的内存空间，并进行了修改

cout << "*p : " << *p << endl; // 30
cout << "a : " << a << endl;  // 10      // 直接替换常量
cout << "*&a : " << *&a << endl; // 10   // 直接替换常量
// &a 和 p 的值都为变量a的地址
cout << "&a : " << &a << endl; // &a
cout << "p : " << p << endl;   // &a

```
编译器直接将a替换为字面值10，不会为其分配内存（除非取地址操作触发临时内存分配）。
示例中，`cout << a`被优化为`cout << 10`，因此即使通过指针修改内存，a的输出仍为10，而*p显示修改后的值30。

- 2. 用变量初始化时（运行时常量）
退化为C风格的只读变量, 也称常变量
```c
int b = 10;
const int a = b;   // 运行时确定常量的值 
int *p = (int *)&a;
*p = 30;
printf("%d", a);   // 30   可以通过指针间接修改
printf("%d", *p);  // 30
```
此时a的值在运行时确定，行为与C语言一致。通过指针修改其内存后，a和*p均输出30。

编译器会为a分配内存空间，且允许通过指针间接修改（尽管语法上不建议）。

### 关键差异总结
| 特性         | C语言                     | C++（常量初始化时）               |
|--------------|--------------------------|----------------------------------|
| 本质         | 运行时只读变量            | 编译时常量（可能触发宏替换）      |
| 内存分配     | 始终分配内存              | 无内存分配（除非取地址操作）      |
| 指针修改     | 允许通过指针间接修改      | 修改临时内存，不影响常量值        |
| 类型安全     | 弱（允许const到非const转换） | 强（需显式强制类型转换）          |
| 应用场景     | 无法用于数组长度等常量表达式 | 可替代`#define`定义常量             |
# 高级

## const引用绑定右值

### 回顾左值，右值，临时变量
#### 左值（Lvalue）
- 定义
  - 左值指具有明确内存地址的表达式，通常可以出现在赋值运算符=的左侧。其名称中的"L"最初表示"Left"，但更准确的理解是"Location"（可寻址）。
- 核心特性
  - 可寻址：可通过&运算符获取地址（如&a）。
  - 持久性：生命周期较长，如变量、对象或数组元素。
  - 可修改性：非const左值可被赋值（如a = 10）。

#### 右值（Rvalue）
- 定义
  - 右值指临时数据或无法寻址的表达式，通常出现在赋值运算符右侧。其名称中的"R"可理解为"Read"（仅可读）或"Register"（可能存储在寄存器中）。
- 核心特性
  - 不可寻址：无法通过&获取地址（如&(a + b)会报错）。
  - 临时性：生命周期短暂，如字面量或表达式结果。
  - 不可修改：不能直接赋值（如10 = a无效）。
- 常见示例
  - 字面量：int x = 5;中的5。
  - 算术表达式结果：a + b。
  - 函数返回非引用值：int func()的返回值。
  - **临时变量**

##### 临时变量为右值的示例
```c
string func() { return string("aaa"); } 
```
- 按值返回的临时对象
  - 该函数通过 `return string("aaa")` 按值返回一个临时构造的 `string` 对象。
  - 根据 C++ 的规则，传值返回的函数会生成一个临时对象，而临时对象属于右值。
- 右值的定义与特性
  - 右值表示临时对象或无法取地址的值（如字面量、表达式结果等）。
  - 此处的 `string("aaa")` 是一个显式构造的临时对象，其生命周期仅限于函数返回时生成的临时对象，无法通过取地址操作获取其内存位置。

```c
auto&& r1 = func();  // 合法：右值引用可以绑定到右值
string s1 = func();  // 合法：右值用于初始化对象（可能触发移动构造）
string& s2 = func(); // 非法：左值引用不能绑定到右值
```
该函数的返回值是右值，适用于移动语义优化场景（如被右值引用接收或触发移动构造函数）。

### const和右值
### const引用/指针绑定右值
```cc
int * const &p = (int *)0x00000000;
```
- 引用类型：`int * const &` 表示一个对常量指针的引用.
- 右值绑定：右侧的 `(int *)0x00000000` 是一个右值（临时指针），而C++允许常量左值引用（const &）绑定到右值，这是语言规则的特殊设计。
- 匿名变量生成：由于右值本身没有内存地址，编译器会隐式创建一个匿名栈变量来存储 0x00000000，然后将引用 p 绑定到此匿名变量。

               const p            匿名变量
           ┌───────────┐      ┌───────────────┐
           │           ├─────►│  0x00000000   │
           │           │      │               │
           └───────────┘      └───────────────┘

```c
// 以下示例都是常引用，都会生成临时对象

// 引用非指针类型时，const int和 int const语义相同，const都是修饰 b 本身
const int &b = 0;
int const &b = 0;

// 引用指针类型时，const int *和 int *const语义不相同
int *const &a = (int *)0x0;
```

## 复杂情况下const的语法推导
### 加const
```cc
    // 指针
    {
        // 一级指针，加const
        int a = 10;
        int* const p1 = &a; // Y 
        const int* p2 = &a; // Y
    }
    {
        // 二级指针
        int *a = (int *)0x00;
        int * * const p1 = &a; // Y : int ** const = int **
        int * const * p2 = &a; // Y : int * const * = int **
        int const * * p3 = &a; // N : int const ** != int **
        int const * * p4 = (int const **)&a; // Y : int const ** = int const **
    }
    {
        // 三级指针
        int **a;
        int *** const p1 = &a;  // Y
        int ** const * p2 = &a; // Y
        int * const ** p3 = &a; // N
        int const *** p4 = &a;  // N
    }
    {
        // 四级指针
        int ***a;
        int **** const p1 = &a;  // Y
        int *** const * p2 = &a; // Y
        int ** const ** p3 = &a; // N
        int * const *** p4 = &a; // N
        int const **** p5 = &a;  // N
    }
    // 结论:
    // 1. 一级指针怎么加const 都可以
    // 2. 多级指针，只有最内层的一级部分可以任意加const，其他层必须严格匹配

    // 引用
    {
        // 一级
        int a = 10;
        int & const p1 = a; // N:  语法错误：&变量之间不能加const
        int const &p2 = a;  // Y: int const * = int *
        const int &p3 = a;  // Y: const int * = int *
    }
    {
        // 二级
        int *a;
        int * const &p1 = a; // Y: int * const * = int **
        int const * &p2 = a; // N: int const * * = int **
    }
    {
        // 三级
        int **a;
        int ** const &p1 = a; // Y: int ** const * = int ***
        int *const * &p2 = a; // N: int * const ** = int ***
        int const ** &p3 = a; // N: int const ***  = int ***
    }
    // 结论:
    // 1. &变量之间不能加const
    // 2. 先将&转换为指针, 规则边和指针一样，只有最内层的一级部分可以任意加const, 其他层必须严格匹配
```

### 去const
```cc

// 去const
    // 指针
    {
        int *q;
        int *const p1 = nullptr;
        q = p1; // int * = int * const
        int const *p2 = nullptr;
        q = p2; // int * != const int *
    }
    {
        int **q;
        int ** const  p1 = nullptr;
        q = p1; // int ** = int **const
        int *const *  p2 = nullptr;
        q = p2; // int ** != int *const*
        int const **  p3 = nullptr;
        q = p3; // int ** != int const **
    }

    {
        int ***q;
        int *** const p1 = nullptr;
        q = p1; // int *** = int *** const
        int **const * p2 = nullptr;
        q = p2; // int *** != int **const*
        int *const ** p3 = nullptr;
        q = p3; // int *** != int *const**
        int const *** p4 = nullptr;
        q = p4; // int *** != int const***
    }
    // 引用
    {
        int *const p1 = nullptr;
        int * &q1 = p1; // int ** != int * const *
        int * const &q1_ = p1; // int * const * == int * const *
        int const *p2 = nullptr;
        int * &q2 = p2; // int ** != int const **
        int const * &q2_ = p2; // int ** == int const **
    }
    {
        int ** const  p1 = nullptr;
        int ** &q1 = p1;// N: int *** != int ** const *
        int ** const &q1_ = p1; // Y: int **const * == int **const *

        int *const *  p2 = nullptr;
        int ** &q2 = p2; // N: int *** != int *const **
        int *const * &q2_ = p2; // Y: int *** == int *const **

        int const **  p3 = nullptr;
        int ** &q3 = p3; // N: int *** != int const ***
        int const ** &q3_ = p3;// Y: int const *** == int const ***
    }
    // 结论
    // 1. 去const时，const只要修饰指向的值，其他层必须严格匹配
```

## 右值引用与移动语义
在 C++ 里，const引用绑定右值和右值引用绑定右值存在明显差异，下面从多个方面进行对比：
- 1. 语法表现
```cc
const T& ref1 = T();    // const左值引用绑定右值
T&& ref2 = T();         // 右值引用绑定右值
```
- 2. 生命周期的延长
  - const 引用：会延长右值的生命周期，使其和引用自身的生命周期保持一致。
  - 右值引用：同样能延长右值的生命周期，不过它还能表明对象具备 “可移动” 的语义。
- 3. 可修改性
  - const 引用：被引用的对象不能被修改，因为引用带有const限定符。
  - 右值引用：可以对被引用的对象进行修改，这为实现移动语义创造了条件。
- 4. 主要用途
  - const 引用：主要用于以只读的方式传递参数，这样能避免不必要的拷贝。
  - 右值引用：主要用于实现移动语义和完美转发。

```cc
#include <iostream>
#include <string>

class Example {
public:
    Example() { std::cout << "构造函数" << std::endl; }
    Example(const Example&) { std::cout << "拷贝构造函数" << std::endl; }
    Example(Example&&) noexcept { std::cout << "移动构造函数" << std::endl; }
    ~Example() { std::cout << "析构函数" << std::endl; }
};

void byConstRef(const Example& e) {
    std::cout << "通过const引用调用" << std::endl;
    // e 不能被修改
}

void byRValueRef(Example&& e) {
    std::cout << "通过右值引用调用" << std::endl;
    Example moved = std::move(e); // 调用移动构造函数
}

int main() {
    std::cout << "=== const引用绑定右值 ===" << std::endl;
    byConstRef(Example()); // 右值被const引用绑定
    
    std::cout << "\n=== 右值引用绑定右值 ===" << std::endl;
    byRValueRef(Example()); // 右值被右值引用绑定并移动
    
    return 0;
}
```

```bash
=== const引用绑定右值 ===
构造函数
通过const引用调用
析构函数

=== 右值引用绑定右值 ===
构造函数
通过右值引用调用
移动构造函数
析构函数
析构函数
```


| 特性             | const T& 绑定右值 | T&& 绑定右值 |
|------------------|-------------------|--------------|
| 语法形式         | const T& = T()    | T&& = T()    |
| 对象可修改性     | 不可修改          | 可以修改     |
| 主要用途         | 避免拷贝          | 实现移动语义 |
| 生命周期延长     | 是                | 是           |
| 能否调用移动构造函数     | 否                | 是           |


## 智能指针的const应用
`std::unique_ptr<const T>`表示指向常量对象的独占指针，比原始指针更安全。

#### const修饰智能指针本身还是指向的对象
智能指针主要有`std::unique_ptr`、`std::shared_ptr`和`std::weak_ptr`这三种。当它们与const结合时，有两种不同的限定情况。

- 1. 指针本身是const（顶层const）
这意味着智能指针一旦指向某个对象，就不能再指向其他对象了，但它所指向的对象是可以被修改的。
```cc
std::unique_ptr<int> const p1 = std::make_unique<int>(42);
// p1 = nullptr;  // 错误，不能给const指针赋值
*p1 = 99;         // 正确，可以修改所指对象的值

std::shared_ptr<int> const p2 = std::make_shared<int>(100);
// p2 = nullptr;  // 错误
*p2 = 200;        // 正确
```

- 2. 所指向的对象是const（底层const）
这表示智能指针所指向的对象不能被修改，但指针本身可以指向其他对象。
```cc
std::unique_ptr<const int> p3 = std::make_unique<int>(42);
// *p3 = 99;      // 错误，不能修改const对象
p3 = std::make_unique<int>(100);  // 正确，可以重新赋值指针

std::shared_ptr<const int> p4 = std::make_shared<int>(100);
// *p4 = 200;     // 错误
p4 = std::make_shared<int>(200);  // 正确
```

- 3. 指针和所指对象都是const
这种情况下，智能指针既不能指向其他对象，它所指向的对象也不能被修改。
```cc
const std::unique_ptr<const int> p5 = std::make_unique<int>(42);
// p5 = nullptr;  // 错误
// *p5 = 99;      // 错误

const std::shared_ptr<const int> p6 = std::make_shared<int>(100);
// p6 = nullptr;  // 错误
// *p6 = 200;     // 错误
```

#### 二、const方法与智能指针
在类的const方法里使用智能指针时，要保证不会修改对象的状态。
```cc
class MyClass {
private:
    std::shared_ptr<int> data;
public:
    MyClass(int value) : data(std::make_shared<int>(value)) {}
    
    // const方法：不能修改对象的非mutable成员
    int getValue() const {
        // *data = 100;  // 错误，在const方法里不能修改data
        return *data;   // 正确，只读访问
    }
    
    // 非const方法：可以修改对象状态
    void setValue(int value) {
        *data = value;  // 正确
    }
};
```

#### 三、传递智能指针参数时的const应用
在函数参数中使用智能指针时，const的使用方式会影响到函数对指针和所指对象的操作权限。

- 1. 按值传递智能指针
这种方式会发生所有权的转移（针对unique_ptr）或者引用计数的增加（针对shared_ptr）。
```cc
void processUnique(std::unique_ptr<int> ptr) {
    // ptr现在拥有对象的所有权
    *ptr = 100;  // 可以修改所指对象
}  // ptr离开作用域，对象被销毁

void processShared(std::shared_ptr<int> ptr) {
    // 引用计数加1
    *ptr = 200;  // 可以修改所指对象
}  // 引用计数减1
```

- 2. 传递const智能指针引用
这种方式可以避免拷贝，同时保证指针本身不会被修改。
```cc
void processUniqueRef(const std::unique_ptr<int>& ptr) {
    // *ptr = 100;  // 正确，可以修改所指对象
    // ptr = nullptr;  // 错误，不能修改const引用
}

void processSharedRef(const std::shared_ptr<int>& ptr) {
    // *ptr = 200;  // 正确
    // ptr = nullptr;  // 错误
}
```
- 3. 传递指向const对象的智能指针
使用这种方式传递参数，能确保所指对象不会被修改。
```cc
void processConstUnique(const std::unique_ptr<const int>& ptr) {
    // *ptr = 100;  // 错误，不能修改const对象
    // ptr = nullptr;  // 错误，不能修改const引用
}

void processConstShared(const std::shared_ptr<const int>& ptr) {
    // *ptr = 200;  // 错误
    // ptr = nullptr;  // 错误
}
```
#### 四、const与shared_ptr的引用计数
当shared_ptr被const修饰时，它的引用计数机制不会受到影响，引用计数的变化只和对象的所有权有关。
```cc
std::shared_ptr<int> p1 = std::make_shared<int>(42);  // 引用计数为1
const std::shared_ptr<int> p2 = p1;                    // 引用计数为2
// p2 = nullptr;  // 错误，p2是const
std::shared_ptr<int> p3 = p2;                          // 引用计数为3，p2的const不影响
```

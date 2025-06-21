# new
## new 和 malloc 的区别
- 初始化行为
  - `malloc`仅分配内存，不调用构造函数，返回未初始化的内存块。
  - `new`分配内存后自动调用构造函数（对类对象），支持初始化表达式（如`new int(5)`）。
```cpp
A* pa = new A(3);  // 分配内存并调用构造函数
A* pa_malloc = (A*)malloc(sizeof(A));  // 仅分配内存，需手动构造
```
- 失败处理机制
  - `malloc`通过返回`NULL`表示失败，需手动检查。
  - `new`默认抛出`std::bad_alloc`异常，支持`nothrow`版本返回`nullptr`。
```cpp
int* p = new (nothrow) int(0);  // 失败返回nullptr
```
- 类型安全
  - `new`直接返回目标类型指针，无需转换。
  - `malloc`返回`void*`，需强制类型转换。

## new的三种形态

- New Operator（new表达式）
```cpp
// 最常见的用法，完成三步操作：
// 1. 调用`operator new`分配内存
// 2. 调用构造函数
// 3. 返回类型化指针
A* pa = new A(3);
```
- Operator New（内存分配函数）
  - 负责纯内存分配，可重载（类内或全局）。默认实现调用`malloc`，失败时触发`new_handler`。
```cpp
void* operator new(size_t size) {
    cout << "Custom allocation"; 
    return malloc(size);
}
```
- Placement New（定位new）
  - 在已分配的内存上构造对象，不分配新内存。常用于内存池或特定地址初始化。
```cpp
char buffer[sizeof(A)];
A* pa = new (buffer) A(3);  // 在buffer上构造对象
```

## new的高级用法
- 数组分配
  - 使用`new[]`分配数组，支持初始化列表或默认初始化。
```cpp
int* arr = new int[10]{1,2,3};  // 前三个元素初始化，其余为0
```
- 常量对象
  - 分配`const`对象需在初始化时赋值。
```cpp
const int* p = new const int(40);  // 必须初始化
```
- 不抛出异常的nothrow版本
  - 通过`new (nothrow)`抑制异常，需检查指针有效性。

- operator new的重载与配对
  - 类内重载
    - 可自定义类的内存分配策略（如日志记录）。
```cpp
class A {
public:
    void* operator new(size_t size) {
        cout << "Allocating A";
        return ::operator new(size);  // 调用全局operator new
    }
};
```
- 全局重载
  - 重载全局`operator new`需谨慎（影响所有动态分配）。需同时重载`operator delete`以保证一致性。

# delete
- `delete`操作符行为：先调用析构函数 → 释放内存（通过`operator delete`）。
- 若重载`operator new`，建议同步重载`operator delete`。
- 数组需使用`delete[]`，否则可能引发内存泄漏或未定义行为。

## delete [] 的底层实现
- 运算符重载：
  - `new` 调用 `operator new`（内部使用 `malloc` 分配内存），失败抛出异常。
  - `delete` 调用 `operator delete`（内部使用 `free` 释放内存）。
  - 可全局重载 `operator new` 和 `operator delete`（如实现内存池）。
- 内存记录机制：
  - 数组分配时（如 `new T[n]`），编译器多分配 4 字节记录元素个数，确保 `delete[]` 正确调用析构次数。
  - 用户指针返回的是数组首元素地址（非实际分配起始地址）。


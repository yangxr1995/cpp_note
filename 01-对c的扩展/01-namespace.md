# 为什么要命名空间
大型程序是由多人合作开发，每个人负责独立的模块，最后组合成一个完整程序，容易出现符号名冲突（类/函数/全局变量），为了解决这个问题，cpp引入了命名空间，命名空间就是用户定义的作用域，不同作用域中可以定义名称相同的符号，不会冲突。

# 命名空间的使用方式
## 命名空间定义
### 嵌套定义
```cc
namespace A {
    namespace B {
        int a;
    }
}

int main (int argc, char *argv[]) {
    A::B::a = 1;
    return 0;
}
```
### 匿名命名空间
```cc
#include <iostream>
using namespace std;
namespace  {
    int a = 10;
}

int main (int argc, char *argv[]) {
    cout << a << endl;   // 10
    cout << ::a << endl; // 10
    return 0;
}
```

```cc
#include <iostream>
using namespace std;
namespace  {
    int a = 10;
}
int a = 100;

int main (int argc, char *argv[]) {
    // cout << a << endl; // 发生冲突
    cout << ::a << endl;  // 100
    return 0;
}

```

### 多次定义命名空间
```cc
namespace aa {
    int a;
}
namespace aa {
    int a; // ERROR
    int b;
    void print() {}
}
namespace aa {
    int c = a;
    print(); // ERROR
}

```
- 命名空间可以定义多次，同名命名空间会合并
- 同名命名空间的实体只能定义一次
- 命名空间中只能定义实体，不能调用函数实体

## 实体导入
### 作用域限定符
每次使用命名空间的实体时，都加上作用域限定符。
```cc
namespace test {
    int a;
}
int main (int argc, char *argv[]) {
    test::a = 0;
    return 0;
}
```
好处：精准
坏处：繁琐

### using
使用using将命名空间所有实体导入，容易导致冲突
```cc
#include <iostream>
using namespace std;

void cout() { // 发生冲突
}

int main (int argc, char *argv[]) {
    cout << "aa" << endl;
    return 0;
}
```
### using声明
不是导入所有实体，而是使用什么就导入什么。
```cc
#include <iostream>
using std::cin;

void cout(int a) {
}

int main (int argc, char *argv[]) {
    int a;
    cin >> a;
    cout(a);
    return 0;
}
```

### 跨模块调用
file1.cc
```cc
// 一个模块中定义
namespace wd {
   int num = 10; 
}
```
main.cc
```cc
#include <iostream>
using namespace std;

// 引用的模块，需要写namespace
namespace wd {
    extern int num;
}

int main (int argc, char *argv[]) {
    cout << wd::num << endl;

// 导入wd::num实体
using wd::num;
    cout << num << endl;

    return 0;
}
```

命名空间中中实体的跨模块调用时，要在新的源文件中再次定义同名的命名空间，进行链接时，两次定义的同名命名空间被认为是同一个命名空间。

#### 匿名命名空间的实体只能在本模块中调用
file1.cc
```cc
// 一个模块中定义
namespace {
   int num = 10; 
}
```
main.cc
```cc
#include <iostream>
using namespace std;

// 引用的模块，需要写namespace
namespace {
    extern int num;
}

int main (int argc, char *argv[]) {
    cout << ::num << endl; // ERROR: 找不到定义
    return 0;
}
```
# 总结
- 命名空间的作用
  - 避免命名冲突：命名空间将全局作用域划分为更小的作用域，避免命名冲突
  - 组织代码：将相关实体放到同个命名空间
  - 版本控制：不同版本的实体放到不同版本的命名空间

- 使用使用命名的指导原则
  - 提倡在有名命名空间中定义变量，而不是在全局作用域中定义变量
  - 如果开发一个函数库或类库，提倡将其放到一个命名空间
  - 对于using的声明，首先将作用域设置为局部而不是全局
  - 不要在头文件中使用using
  - 包含头文件的顺序可能会影响程序的行为，如果非要使用using，建议放到所有include后





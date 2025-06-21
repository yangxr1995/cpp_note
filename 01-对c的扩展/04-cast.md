# static_cast
最常用的类型转换符，在正常状况下的类型转换, 用于将一种数据类型转换成另一种数据类型，如把int转换为float
```cc
int iNumber = 100；
float fNumber = 0；
fNumber = (float) iNumber；
fNumber = static_cast<float>(iNumber);

void * pVoid = malloc(sizeof(int));
int * pInt = static_cast<int*>(pVoid);
*pInt = 1;
```
但不能完成任意两个指针类型间的转换
```cc
int iNumber = 1;
int * pInt = &iNumber;
float * pFloat = static_cast<float *>(pInt);//error
```
总结，static_cast的用法主要有以下几种：
- 用于基本数据类型之间的转换，如把int转换成char，把int转换成enum。这种转换的安全需要开发人员来保证；
- 把void指针转换成目标类型的指针，但不安全；
- 把任何类型的表达式转换成void类型；
- 用于类层次结构中基类和子类之间指针或引用的转换

# const_cast
该运算符用来修改类型的const属性，基本不用。
常量指针被转化成非常量指针，并且仍然指向原来的对象；
常量引用被转换成非常量引用，并且仍然指向原来的对象；
常量对象被转换成非常量对象。
```cc
const int number = 100;
int * pInt = &number;//error
int * pInt2 = const_cast<int *>(&number);
```

# dynamic_cast
dynamic_cast：该运算符主要用于基类和派生类间的转换，尤其是向下转型的用法中

# reinterpret_cast
reinterpret_cast：功能强大，慎用（也称为万能转换）
该运算符可以用来处理无关类型之间的转换，即用在任意指针（或引用）类型之间的转
换，以及指针与足够大的整数类型之间的转换。由此可以看出，reinterpret_cast的效果很
强大，但错误的使用reinterpret_cast很容易导致程序的不安全，只有将转换后的类型值转
换回到其原始类型，这样才是正确使用reinterpret_cast方式。


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

dynamic_cast 是 C++ 的一种类型转换运算符，主要用于处理运行时类型识别（RTTI），在类的继承层次结构中进行安全的向下转换或跨转换。以下是其典型应用场景及示例：

## 1. 安全的向下转换（基类指针 / 引用 → 派生类指针 / 引用）
当你持有一个基类指针或引用，但需要访问派生类的特有成员时，dynamic_cast 会在运行时检查类型的有效性。若转换失败：
指针类型返回 `nullptr`。
引用类型抛出 `std::bad_cast` 异常。
```cc
#include <iostream>
class Shape {
public:
    virtual ~Shape() = default; // 必须有虚函数，RTTI 才生效
};

class Circle : public Shape {
public:
    void drawCircle() { std::cout << "Drawing circle...\n"; }
};

class Square : public Shape {
public:
    void drawSquare() { std::cout << "Drawing square...\n"; }
};

void renderShape(Shape* shape) {
    if (Circle* circle = dynamic_cast<Circle*>(shape)) {
        circle->drawCircle(); // 安全转换为 Circle*
    } else if (Square* square = dynamic_cast<Square*>(shape)) {
        square->drawSquare(); // 安全转换为 Square*
    } else {
        std::cout << "Unknown shape type.\n";
    }
}

int main() {
    Circle circle;
    Shape* shapePtr = &circle; // 基类指针指向派生类对象
    renderShape(shapePtr);     // 输出: Drawing circle...
}
```

## 2. 交叉转换（兄弟类之间的转换）
当两个派生类继承自同一个基类，dynamic_cast 可以安全地将一个派生类指针 / 引用转换为另一个派生类的指针 / 引用。
```cc
class Shape { virtual ~Shape() {} };
class Circle : public Shape { /* ... */ };
class ColoredCircle : public Circle {
public:
    void setColor(int color) { std::cout << "Color set to " << color << "\n"; }
};

void processColoredCircle(Shape* shape) {
    // 从基类指针转换到兄弟类指针
    if (ColoredCircle* cc = dynamic_cast<ColoredCircle*>(shape)) {
        cc->setColor(255);
    }
}
```
## 3. 引用类型的转换（处理异常）
与指针不同，引用不能为 nullptr，因此转换失败时会抛出异常。
```cc
void printCircleArea(const Shape& shape) {
    try {
        const Circle& circle = dynamic_cast<const Circle&>(shape);
        // 使用 circle 对象...
    } catch (const std::bad_cast& e) {
        std::cerr << "Error: Not a circle object.\n";
    }
}
```

## 注意事项
- 虚函数必须存在：dynamic_cast 依赖于虚函数表（VTBL）实现 RTTI，因此基类必须有至少一个虚函数（通常是虚析构函数）。
- 性能开销：运行时类型检查会带来额外开销，频繁使用可能影响性能。
- 替代方案：优先考虑使用虚函数实现多态，仅在设计上确实需要类型转换时使用 dynamic_cast。

## 总结
dynamic_cast 的核心价值在于安全地突破静态类型系统的限制，但需谨慎使用，避免破坏面向对象设计的原则（如依赖倒置）。

# reinterpret_cast
reinterpret_cast：功能强大，慎用（也称为万能转换）
该运算符可以用来处理无关类型之间的转换，即用在任意指针（或引用）类型之间的转
换，以及指针与足够大的整数类型之间的转换。由此可以看出，reinterpret_cast的效果很
强大，但错误的使用reinterpret_cast很容易导致程序的不安全，只有将转换后的类型值转
换回到其原始类型，这样才是正确使用reinterpret_cast方式。


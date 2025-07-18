# C++ 继承核心要点总结

以下是关于 C++ 继承核心要点的总结，结合课程内容与网络资料。

---

## 二、继承的访问权限

### 1. 继承方式与权限变化

| 基类成员权限 | 公有继承（public） | 保护继承（protected） | 私有继承（private） |
|--------------|-------------------|----------------------|-------------------|
| public       | public            | protected            | private           |
| protected    | protected         | protected            | private           |
| private      | 不可访问          | 不可访问             | 不可访问          |

- **公有继承**：基类公有和保护成员在派生类中保持原权限。
- **保护继承**：基类公有和保护成员在派生类中变为 `protected`，外部无法直接访问。
- **私有继承**：基类成员在派生类中变为 `private`，阻断进一步继承。

### 2. 默认继承方式
- `class` 定义：默认私有继承（`private`）。
- `struct` 定义：默认公有继承（`public`）。

---

## 三、派生类的构造与析构

### 1. 构造顺序
先调用基类构造函数，再调用派生类构造函数。

### 2. 析构顺序
先调用派生类析构函数，再调用基类析构函数。

### 3. 派生类如何初始化基类
若基类构造函数带参数，需通过派生类构造函数的初始化列表显式调用：
```cpp

class Animal {
    public:
     Animal(int color) :_color(color) {}
     int _color;
};

class Cat : public Animal {
    public:
     Cat(int color) : Animal(color) {} // 正确
     Cat(int color) : _color(color) {} // 错误
};
```
---

## 重载和隐藏
- 重载: 同一作用域，函数名相同，参数列表不同，构成重载
- 隐藏: 继承时，派生类函数将覆盖和基类函数名相同的函数,若要强制调用基类方法，必须加基类作用域
```c
class Base {
    public:
        void show() {}
        void show(int a) {}
};
// 场景1
class Derive : public Base {
    public:
};
// 派生类继承了基类的所有方法
Derive d;
d.show();
d.show(10);

class Derive2 : public Base {
    public:
        void show() {}
};
Derive2 d2;
d2.show(); // 调用 Derive2::show
d2.show(10); // 错误 Base::show(int)被隐藏了
d2.Base::show(10); // 正确
```

---

## 基类对象和派生类对象的互相转换
基类对象和派生类对象的互相转换默认只能从上到下
```c
Base b;
Derive d;

// 派生类转换为基类，可行
b = d;

// 基类转换为派生类对象，不可行
d = b;

// 基类引用指向派生类，可行
Base &pb = d;

// 基类指针指向派生类，可行
Base *pb2 = d;

// 派生类引用基类对象，不可行
Derive &pd = b;
```

---

## 动态绑定和静态绑定
### 编译器如何确定使用动态绑定还是静态绑定
当编译通过指针或引用调用函数的语句时，检查函数是否为virtual，若是则用动态绑定，否则用静态绑定

```c
// 检查 Base::show() 是否为virtual
Base *pb = &d;
pb->show();     // void Base::show()
```

### 静态绑定
编译阶段确定链接地址
```c
class Base {
    public:
        void show() { cout << __PRETTY_FUNCTION__ << endl;}
        void show(int a) {cout << __PRETTY_FUNCTION__ << endl;}
        void show(int a, int b) { cout << __PRETTY_FUNCTION__ << endl; }
};

class Derive: public Base {
    public:
        void show() { cout << __PRETTY_FUNCTION__ << endl; }
};

int main (int argc, char *argv[]) {

    Derive d;
    Base *pb = &d;

    pb->show();     // void Base::show()
   // 107cc:	ldr	r0, [fp, #-12]
   // 107d0:	bl	108cc <Base::show()>

    pb->show(10);   // void Base::show(int)
   // 107d4:	mov	r1, #10
   // 107d8:	ldr	r0, [fp, #-12]
   // 107dc:	bl	10910 <Base::show(int)>

    pb->show(1, 1); // void Base::show(int, int)
   // 107e0:	mov	r2, #1
   // 107e4:	mov	r1, #1
   // 107e8:	ldr	r0, [fp, #-12]
   // 107ec:	bl	10958 <Base::show(int, int)>

    cout << typeid(pb).name() << endl;  // P4Base
    cout << typeid(*pb).name() << endl; // 4Base
    
    return 0;
}
```

### 动态绑定

```c
class Base {
    public:
        virtual void show() { cout << __PRETTY_FUNCTION__ << endl;}
        virtual void show(int a) {cout << __PRETTY_FUNCTION__ << endl;}
        void show(int a, int b) { cout << __PRETTY_FUNCTION__ << endl; }
};

class Derive: public Base {
    public:
        void show() { cout << __PRETTY_FUNCTION__ << endl; }
};

int main (int argc, char *argv[]) {

    Derive d;
    Base *pb = &d;

    pb->show();     // virtual void Derive::show()
   // 1085c:	ldr	r3, [fp, #-12]  ; 取pb的值(d的地址)
   // 10860:	ldr	r3, [r3]        ; pb解引用，获得vptr(d第一项属性)
   // 10864:	ldr	r3, [r3]        ; vptr解引用，获得virtual void show() 的地址
   // 10868:	ldr	r0, [fp, #-12]  ; 传实参this
   // 1086c:	blx	r3

    pb->show(10);   // virtual void Base::show(int)
   // 10870:	ldr	r3, [fp, #-12]  ; 取pb的值(d的地址)
   // 10874:	ldr	r3, [r3]        ; pb接引用，获得vptr(d的第一项属性)
   // 10878:	add	r3, r3, #4      ; vptr偏移4字节获得virtual void show(int)的指针的地址
   // 1087c:	ldr	r3, [r3]        ; 获得virtual void show(int)的地址
   // 10880:	mov	r1, #10         ; 传实参10
   // 10884:	ldr	r0, [fp, #-12]  ; 传实参this
   // 10888:	blx	r3

    pb->show(1, 2); // void Base::show(int, int)
   // 1088c:	mov	r2, #2
   // 10890:	mov	r1, #1
   // 10894:	ldr	r0, [fp, #-12]
   // 10898:	bl	109f8 <Base::show(int, int)>

    cout << typeid(pb).name() << endl;  // P4Base

    cout << typeid(*pb).name() << endl; // 6Derive
   // 108c4:	ldr	r3, [fp, #-12]  ; 获得pb的值
   // 108d0:	ldr	r3, [r3]         ; 获得d的vptr的值
   // 108d4:	ldr	r3, [r3, #-4]    ; vptr-4获得rtti对象
   // 108d8:	mov	r0, r3           ; 传入rtti对象的地址做this指针
   // 108dc:	bl	10964 <std::type_info::name() const> ; 调用rtti的name()
   // 108e0:	mov	r3, r0
   // 108e4:	mov	r1, r3
   // 108e8:	ldr	r0, [pc, #108]	@ 1095c <main+0x134>
   // 108ec:	bl	106f4 <std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*)@plt>
   // 108f0:	mov	r3, r0
   // 108f4:	ldr	r1, [pc, #100]	@ 10960 <main+0x138>
   // 108f8:	mov	r0, r3
   // 108fc:	bl	10700 <std::ostream::operator<<(std::ostream& (*)(std::ostream&))@plt>
 
    return 0;
}
```
### 虚函数表的结构
核心要点
- 基类有虚函数，则派生类继承虚函数.
- 派生类定义的同名同返回值通函数列表的函数可以覆盖自己虚函数表中的基类虚函数.
- 每一个有虚函数的类有一个虚函数表.
- 有虚函数的类对象有一个虚函数表指针。
- 虚函数指针指向自己类的虚函数表的第一个虚函数


     ┌──────────┐                                  ┌─────────────────────────────────────────┐
     │ Base b   │                                  │  class Base的虚函数表                   │
     ├──────────┤                                  ├─────────────────────────────────────────┤
     │   vptr   ├───────────────┐                  │       RTTI(typeinfo)                    │
     └──────────┘               │                  ├─────────────────────────────────────────┤
                                │                  │       0                                 │
                                │                  ├─────────────────────────────────────────┤
                                └─────────────────►│       virtual void Base::show()         │
                                                   ├─────────────────────────────────────────┤
                                                   │       virtual void Base::show(int)      │
                                                   └─────────────────────────────────────────┘


     ┌─────────┐                                   ┌─────────────────────────────────────────┐
     │Derive d │                                   │ class Derive的虚函数表                  │
     ├─────────┤                                   ├─────────────────────────────────────────┤
     │   vptr  ├────────────────┐                  │       RTTI(typeinfo)                    │
     └─────────┘                │                  ├─────────────────────────────────────────┤
                                │                  │       0                                 │
                                │                  ├─────────────────────────────────────────┤
                                └─────────────────►│       virtual void Derive::show()// 覆盖│
                                                   ├─────────────────────────────────────────┤
                                                   │       virtual void Base::show(int)      │
                                                   └─────────────────────────────────────────┘







---

# 虚函数
## 虚函数和静态函数
静态函数不能设置为虚函数，因为虚函数机制依赖虚函数指针，静态函数没有对象，所以没有虚函数指针

## 虚函数和构造函数
调用基类构造函数时，vptr指向基类的vtable中的虚函数表，

调用自己的构造函数时，vptr才指向本类vtable中的虚函数表。

注意派生类的构造函数中可以使用多态，因为本类vptr的绑定紧跟在基类构造返回后.

```cpp
0001095c <Sub::Sub()>:
		Sub() {
   1095c:	e92d4800 	push	{fp, lr}
   10960:	e28db004 	add	fp, sp, #4
   10964:	e24dd010 	sub	sp, sp, #16
   10968:	e50b0010 	str	r0, [fp, #-16]
   1096c:	e51b3010 	ldr	r3, [fp, #-16]
   10970:	e1a00003 	mov	r0, r3
   10974:	ebffffda 	bl	108e4 <Base::Base()> ; 调用基类的构造
                                                 ; 基类构造完成进行派生类构造
                                                 ; vptr指向本类vtable
   10978:	e59f2030 	ldr	r2, [pc, #48]	     ; 获取vtable的地址
   1097c:	e51b3010 	ldr	r3, [fp, #-16]       ; 获得vptr的地址
   10980:	e5832000 	str	r2, [r3]             ; 将vptr指向vtable

			b = this;
   10984:	e51b3010 	ldr	r3, [fp, #-16]
   10988:	e50b3008 	str	r3, [fp, #-8]
			b->show();
   1098c:	e51b3008 	ldr	r3, [fp, #-8]        ; 动态绑定
   10990:	e5933000 	ldr	r3, [r3]
   10994:	e5933000 	ldr	r3, [r3]
   10998:	e51b0008 	ldr	r0, [fp, #-8]
   1099c:	e12fff33 	blx	r3
		}
```
## 析构函数和析构函数

如果要使用多态，则基类析构函数推荐使用虚函数，因为若不为虚函数，则可能漏掉调用子类的析构

示例：析构函数不为虚函数时，可能导致内存泄露
```cpp
	Base *b = new Sub;
   10864:	e3a00004 	mov	r0, #4
   10868:	ebffff97 	bl	106cc <operator new(unsigned int)@plt>
   1086c:	e1a03000 	mov	r3, r0
   10870:	e1a04003 	mov	r4, r3
   10874:	e1a00004 	mov	r0, r4
   10878:	eb00005a 	bl	109e8 <Sub::Sub()>    ; 派生类构造
   1087c:	e50b4010 	str	r4, [fp, #-16]
	delete b;
   10880:	e51b4010 	ldr	r4, [fp, #-16]        ; 获得指针b
   1088c:	e1a00004 	mov	r0, r4                
   10890:	eb000047 	bl	109b4 <Base::~Base()> ; 调用基类的析构，导致内存泄露
   10894:	e1a00004 	mov	r0, r4
   10898:	ebffff97 	bl	106fc <operator delete(void*)@plt>
```

示例: 将基类的析构函数设置为虚函数，则析构时能调用到正确的派生类析构函数
```cpp
	Base *b = new Sub;
   10864:	e3a00004 	mov	r0, #4
   10868:	ebffff97 	bl	106cc <operator new(unsigned int)@plt>
   1086c:	e1a03000 	mov	r3, r0
   10870:	e1a04003 	mov	r4, r3
   10874:	e1a00004 	mov	r0, r4
   10878:	eb000068 	bl	10a20 <Sub::Sub()>   ; 派生类构造
   1087c:	e50b4010 	str	r4, [fp, #-16]
	delete b;                                ; 析构函数为虚函数，进行动态绑定
   1088c:	e51b3010 	ldr	r3, [fp, #-16]   ; 获得vptr
   10890:	e5933000 	ldr	r3, [r3]         ; 获得vtable中的虚函数表
   10894:	e2833008 	add	r3, r3, #8       ; 虚函数表基地址偏移8字节得到虚函数的指针
   10898:	e5933000 	ldr	r3, [r3]         ; 获得虚函数
   1089c:	e51b0010 	ldr	r0, [fp, #-16]
   108a0:	e12fff33 	blx	r3               ; 调用派生类的析构函数
```

### 哪些函数不能实现成虚函数
1. 构造函数不能实现为虚函数

因为多态依赖vptr指向对象对应类的vtable，而这个操作在对象的构造函数中完成，

所以构造函数若做虚函数，则vptr还没有指向正确的vtable导致无法调用正确的构造函数。

2. 静态方法不能实现为虚函数

因为多态依赖vptr指针，vptr存储在确定的对象里，而静态方法不需要对象

### 是不是所有虚函数的调用都是动态绑定？
不是，比如通过对象直接调用虚函数时，是静态绑定，在构造函数中直接调用虚函数，也是静态绑定. 

只有通过指针或引用访问函数，且函数为虚函数，才会编译为动态绑定，否则是静态绑定
```c
Base b;
Derive d;
b.show();  // 静态绑定
d.show();

Base *pb = &d;
pb->show(); // 动态绑定

Base &bb = &d;
bb.show(); // 动态绑定

Derive &pd = (Derive *)&b;
pd->show();  // 动态绑定 Base::show()
```


### 虚函数和默认参数

注意多态时默认参数使用的是基类的指定的默认参数。
```cpp
class Base {
	public:
		virtual void show(int i = 10) { cout << "Base::show " << i << endl;}
		virtual ~Base() {}
};

class Derive: public Base {
	public:
		virtual void show(int i = 20) { cout << "Derive::show " << i << endl;}
		virtual ~Derive() {}
}; 

	Base *p = new Derive;
	p->show(); // Derive::show 10
   // 10838:	e51b3010 	ldr	r3, [fp, #-16]
   // 1083c:	e5933000 	ldr	r3, [r3]
   // 10840:	e5933000 	ldr	r3, [r3]
   // 10844:	e3a0100a 	mov	r1, #10   ; 使用默认参数做参数
   // 10848:	e51b0010 	ldr	r0, [fp, #-16]
   // 1084c:	e12fff33 	blx	r3

	delete p;
```

原因是，使用默认参数传参，是编译时确定，编译器无法确定动态绑定那个函数，

所以编译器完全按照调用时当时的对象是什么类型，就使用对应类型定义的函数声明的默认参数。

### 虚函数和访问域控制
```cpp
class Base {
	public:
		virtual void show(int i = 10) { cout << "Base::show " << i << endl;}
		virtual ~Base() {}
};

class Derive: public Base {
	private:
		virtual void show(int i = 20) { cout << "Derive::show " << i << endl;}
	public:
		virtual ~Derive() {}
}; 

int main()
{
	Base *p = new Derive;

	p->show();
	delete p;

	return 0;
}
```
虽然Derive::show是private，但是依旧可以访问，

因为访问域是在编译阶段判定，编译器发现 p 是 Base，且`Base::show`是 public，所以可以调用。运行阶段找到`Derive::show`，并运行

---

## 抽象类
### 什么时候定义抽象类
- 为了定义一个统一的接口
- 为了继承公共属性

### 抽象类的特点
- 抽象类方法没有明确实现，只有接口定义 
- 抽象类不能定义对象，但可以定义指针和引用

---

## 多继承和虚继承
### 多继承的问题
多继承可以带来更多代码复用的好处，但是存在冗余的问题

             ┌────────┐                                      ┌─────────┐
             │class A │                                      │ class A │
             │    _a  │                                  ┌──►│     _a  │
             └────────┘                                  │   └─────────┘
                 ▲ ▲                                     │         ▲
          ┌──────┘ └──────┐                              │         │
    ┌─────┴─────┐    ┌────┴─────┐                        │   ┌─────┴──────┐
    │class B    │    │class C   │                        │   │ class B    │
    │    class A│    │   class A│                        │   │     class A│
    │        _a │    │      _a  │                        │   │         _a │
    │    _b     │    │   _c     │                        │   │     _b     │
    └───────────┘    └──────────┘                        │   └────────────┘
          ▲               ▲                              │         ▲
          └────────┐ ┌────┘                              │         │
            ┌──────┴─┴──────┐                            │   ┌─────┴─────────┐
            │ class D       │                            │   │ class C       │
            │    class B    │                            │   │    class A    │
            │        class A│                            └───┤       _a      │
            │            _a │                                │    _c         │
            │        _b     │                                │    class B    │
            │    class C    │                                │        class A│
            │       class A │                                │            _a │
            │          _a   │                                │        _b     │
            │       _c      │                                └───────────────┘
            └───────────────┘

上面示例中，最下层派生类都多了一份 class A的属性。


解决方法是使用虚继承

              ┌────────┐                                      ┌─────────┐       
              │class A │                                      │ class A │       
              │    _a  │                                  ┌──►│     _a  │       
              └────────┘                                  │   └─────────┘       
    virtual       ▲ ▲  virtual                  virtual   │         ▲           
           ┌──────┘ └──────┐                              │         │ virtual          
     ┌─────┴─────┐    ┌────┴─────┐                        │   ┌─────┴──────┐    
     │class B    │    │class C   │                        │   │ class B    │    
     │    class A│    │   class A│                        │   │     class A│    
     │        _a │    │      _a  │                        │   │         _a │    
     │    _b     │    │   _c     │                        │   │     _b     │    
     └───────────┘    └──────────┘                        │   └────────────┘    
           ▲               ▲                              │         ▲           
           └────────┐ ┌────┘                              │         │           
             ┌──────┴─┴──────┐                            │   ┌─────┴─────────┐ 
             │ class D       │                            │   │ class C       │ 
             │    class B    │                            │   │    class A    │ 
             │        class A│                            └───┤       _a      │ 
             │            _a │                                │    _c         │ 
             │        _b     │                                │    class B    │ 
             │    class C    │                                │        class A│ 
             │       class A │                                │            _a │ 
             │          _a   │                                │        _b     │ 
             │       _c      │                                └───────────────┘ 
             └───────────────┘                                                  




---


## 四、组合与继承的选择

### 1. 组合（Composition）
- 表示“整体-部分”关系
- 通过内嵌对象实现，不直接暴露组件接口。

### 2. 继承（Inheritance）
- 表示“一般-特殊”关系
- 支持多态，可通过虚函数实现运行时动态绑定。

---

## 五、多态与虚函数

### 1. 虚函数机制
- 通过虚函数表（`vtable`）实现动态绑定。
- 基类指针指向派生类对象时，调用实际类型的函数：
```cpp
Shape* shape = new Circle();
shape->draw(); // 调用 Circle::draw()
```

### 2. 纯虚函数与抽象类
- 含纯虚函数（`virtual void draw() = 0;`）的类为抽象类，不可实例化。
- 强制派生类实现接口，增强设计规范性。

---

## 六、多继承与虚基类

### 1. 多继承问题
- 菱形继承可能导致数据冗余（如 `D` 继承 `B` 和 `C`，而 `B` 和 `C` 均继承 `A`）。
- 通过虚继承解决：虚基类数据仅保留一份副本。

### 2. 虚基类指针（VBPTR）
虚继承时，派生类通过虚基类指针间接访问共享数据。

---

## 七、常见面试问题

### 1. 覆盖（Override） vs 隐藏（Hide）
- **覆盖**：派生类重写基类虚函数，支持多态。
- **隐藏**：派生类定义同名非虚函数，屏蔽基类函数。


### 2. 静态绑定 vs 动态绑定
- **静态绑定**：编译时确定函数调用（如非虚函数）。
- **动态绑定**：运行时通过虚函数表确定（如虚函数）。

---

# 虚继承
## vbptr vbtable



## 总结
继承是 C++ 实现代码复用和多态的核心机制。合理选择继承方式（公有/保护/私有）、区分组合与继承场景、掌握虚函数与多态原理，是设计高质量面向对象程序的关键。建议结合具体项目实践（如网页的 `Shape-Circle-Rectangle` 案例）加深理解。


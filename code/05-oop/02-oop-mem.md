# gdb 分析c++对象内存布局

## 单继承
本文分析如下2个场景：

没有虚函数的类之间实现单继承；
- 有虚函数的类之间实现单继承。
- 单继承指的是派生类只继承一个基类。

虚函数则是实现C++类对象多态的机制。

### 没有虚函数的类之间实现单继承
以下代码是没有虚函数的类之间实现单继承。

```cc
#include <iostream>
using namespace std;
class Base {
public:
    void func_a() {cout << __PRETTY_FUNCTION__ << endl;};
    void func_b() {cout << __PRETTY_FUNCTION__ << endl;};
    int a = 10;
};
class Derive: public A {
public:
    void func_a() { cout << __PRETTY_FUNCTION__ << endl; };
    void func_b() { cout << __PRETTY_FUNCTION__ << endl; };
    int d = 40;
};

int main() {
    std::cout << "sizeof(A): " << sizeof(A) << std::endl;
    std::cout << "sizeof(D): " << sizeof(D) << std::endl;
    Derive* d = new Derive();
    std::cout << d << std::endl;
    Base* a = d;
    std::cout << a << std::endl;
    // 非虚函数，使用静态绑定
    d->func_a();
   // 107cc:	ldr	r0, [fp, #-20]	@ 0xffffffec
   // 107d0:	bl	10840 <Derive::func_a()>
    a->func_a();
   // 107d4:	ldr	r0, [fp, #-16]
   // 107d8:	bl	107fc <Base::func_a()>
    return 0;
}
```
程序执行结果
```bash
❯ ./a.out
sizeof(A): 4
sizeof(D): 8
0x5910aa4a36c0
0x5910aa4a36c0
void Derive::func_a()
void Base::func_a()
```

#### gdb分析内存布局

```txt
dap> -exec set print pretty on
=cmd-param-changed,param="print pretty",value="on"
dap> -exec p *a
$4 = {
  a = 10
}
dap> -exec p *d
$5 = {
  <A> = {
    a = 10
  }, 
  members of D:
  d = 40
}
```

可见一般类继承后的结构为，基类在低地址，子类在高地址，
基类指针指向子类对象时，实际指向子类对象的基类部分。

                         ┌──────────┐
                         │ Derive   │
       Base *pb────────► │   Base::a│
                         │   d      │
                         └──────────┘

### 有虚函数的类之间实现单继承
以下代码是有虚函数的类之间实现单继承。

```cc
#include <iostream>
using namespace std;
class Base {
public:
    virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;};
    virtual void func_b() {cout << __PRETTY_FUNCTION__ << endl;};
    int _a = 10;
};
class Derive: public Base {
public:
    virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;};
    virtual void func_b() {cout << __PRETTY_FUNCTION__ << endl;};
    int _b = 40;
};
int main() {
    std::cout << "sizeof(Base): " << sizeof(Base) << std::endl;
    std::cout << "sizeof(Derive): " << sizeof(Derive) << std::endl;
    Derive* d = new Derive();
    std::cout << d << std::endl;
    Base* b = d;
    std::cout << b << std::endl;
    d->func_a();
   // 10878:	ldr	r3, [fp, #-20]	@ 0xffffffec
   // 1087c:	ldr	r3, [r3]
   // 10880:	ldr	r3, [r3]
   // 10884:	ldr	r0, [fp, #-20]	@ 0xffffffec
   // 10888:	blx	r3

    b->func_a();
   // 1088c:	ldr	r3, [fp, #-16]
   // 10890:	ldr	r3, [r3]
   // 10894:	ldr	r3, [r3]
   // 10898:	ldr	r0, [fp, #-16]
   // 1089c:	blx	r3

    return 0;
} 
```

```bash
❯ ./a.out
sizeof(Base): 16
sizeof(Derive): 16
0x55555556b6c0
0x55555556b6c0
virtual void Derive::func_a()
virtual void Derive::func_a()
```

除了成员函数会占用类对象的内存空间，还另外有一个指向虚函数表的虚函数表指针（vptr）会占用内存空间，且为了对齐，类对象的内存空间是8的倍数。
经过upcast后的基类指针指向的地址与派生类指针指向的地址是一样的。
调用成员虚函数是根据被调用者本身的类类型决定，也就是说，不管调用者的类类型是什么，最终调用的是被调用者对应的虚函数表里的函数，这就是虚函数的作用，虚函数实现了派生类的多态。
生成的派生类的对象内存布局如下。派生类的生成过程为：首先调用基类的构造函数，此时对象的虚函数表指针指向基类的虚函数表，接着调用派生类的构造函数，此时，一方面，“覆盖”原来的虚函数表指针，使指针指向派生类的虚函数表，另外一方面，增加派生类自己的成员变量到新生成的对象中。

#### gdb分析内存布局

1. set print demangle on
功能：启用后，GDB会将C/C++编译器生成的修饰符号（mangled name）转换为源码中原始的可读名称。例如，将_Z3fooi转换为foo(int)。
适用场景：调试C/C++程序时，查看函数名、类名、变量名等符号的源码形式，避免因编译器修饰导致的名称混乱。
默认状态：默认开启。
2. set print asm-demangle on
功能：启用后，GDB在反汇编代码（如disassemble命令输出）中也会对符号进行反修饰，显示源码中的原始名称。
适用场景：分析汇编指令时，快速定位函数调用或变量地址对应的源码符号。例如，在嵌入式调试中查看ARM汇编指令时，能直接识别main函数而非_Z4mainv。
默认状态：默认关闭，需手动开启。

```txt
dap> -exec set print pretty on
dap> -exec set print asm-demangle on
dap> -exec set print demangle on
dap> -exec p *d
$1 = {
  <Base> = {
    _vptr.Base = 0x555555557d38 <vtable for Derive+16>,
    _a = 10
  }, 
  members of Derive:
  _b = 40
}
dap> -exec p *b
$2 = {
  _vptr.Base = 0x555555557d38 <vtable for Derive+16>,
  _a = 10
}

// 查看对象d的前16字节, 获得vptr指针的值
dap> -exec x/2xg 0x55555556b6c0
0x55555556b6c0:	0x0000555555557d38	0x000000280000000a

// 根据汇编可知vptr指针需要减一个地址长度才能获得vtable项的起始地址
// 所以vptr-8获得Derive 的vtable 项
dap> -exec x/4xg 0x0000555555557d30
0x555555557d30 <vtable for Derive+8>:	0x0000555555557d68	0x000055555555537a
0x555555557d40 <vtable for Derive+24>:	0x00005555555553b8	0x0000000000000000

// vtable第1项为Derive 的 typeinfo的指针
dap> -exec x 0x0000555555557d68
0x555555557d68 <typeinfo for Derive>:	0x00007ffff7e6ebe8

// vtable第2项为Derive::func_a
dap> -exec x 0x000055555555537a
0x55555555537a <Derive::func_a()>:	0xe5894855fa1e0ff3

// vtable第3项为Derive::func_b
dap> -exec x 0x00005555555553b8
0x5555555553b8 <Derive::func_b()>:	0xe5894855fa1e0ff3
```

                                                        ┌───────────────────────┐
                            ┌───────────────┐           │vtable item of Derive  │
     ┌─────────┐            │Derive         │           │    typeinfo of Derive │
     │Base *pb ├─────┬──────┼─► Base        │      ┌────┼──► Derive::func_a()   │
     └─────────┘     │      │     _vptr.Base├──────┘    │    Derive::func_b()   │
     ┌──────────┐    │      │     _a = 10   │           └───────────────────────┘
     │Derive *pd├────┘      │   _b          │
     └──────────┘           └───────────────┘


## 多继承

### 非菱形继承
```cc
#include <iostream>
using namespace std;
class Base_a {
    public:
        virtual void func_a() { cout << __PRETTY_FUNCTION__ << endl;};
        int a = 10;
};
class Base_b {
    public:
        virtual void func_b() { cout << __PRETTY_FUNCTION__ << endl;};
        long b = 20;
};
class Derive: public Base_a, public Base_b {
    public:
        virtual void func_a() { cout << __PRETTY_FUNCTION__ << endl;};
        virtual void func_b() { cout << __PRETTY_FUNCTION__ << endl;};
        int d = 40;
};
int main() {
    std::cout << "sizeof(A): " << sizeof(Base_a) << std::endl; // 16
    std::cout << "sizeof(B): " << sizeof(Base_b) << std::endl; // 16
    std::cout << "sizeof(D): " << sizeof(Derive) << std::endl; // 40
    Derive* d = new Derive();
    std::cout << d << std::endl;     // 0x55555556b6c0
    Base_b* b = d;
    std::cout << b << std::endl;     // 0x55555556b6d0
    Base_a* a = d;
    std::cout << a << std::endl;     // 0x55555556b6c0

    a->func_a();
   // 10910:	ldr	r3, [fp, #-16] ; a
   // 10914:	ldr	r3, [r3]       ; vptr.Base_a
   // 10918:	ldr	r3, [r3]       ; Derive::func_a
   // 1091c:	ldr	r0, [fp, #-16]
   // 10920:	blx	r3

    b->func_b();
   // 10924:	ldr	r3, [fp, #-20]	; b
   // 10928:	ldr	r3, [r3]        ; vptr.Base_b
   // 1092c:	ldr	r3, [r3]        ; Derive::func_b
   // 10930:	ldr	r0, [fp, #-20]	@ 0xffffffec
   // 10934:	blx	r3
// Dump of assembler code for function non-virtual thunk to Derive::func_b():
//    0x0000555555555480 <+0>:	endbr64
//    0x0000555555555484 <+4>:	sub    rdi,0x10  ; 第一个参数this指针(此时指向Derive::Base_b)减0x10，让其指向Derive
//    0x0000555555555488 <+8>:	jmp    0x555555555442 <Derive::func_b()>
// End of assembler dump.

    return 0;
}
```

#### gdb分析内存布局
```txt
dap> -exec p *d
$7 = {
  <Base_a> = {
    _vptr.Base_a = 0x555555557ce0 <vtable for Derive+16>,
    a = 10
  }, 
  <Base_b> = {
    _vptr.Base_b = 0x555555557d00 <vtable for Derive+48>,
    b = 20
  }, 
  members of Derive:
  d = 40
}
dap> -exec x/4xg 0x555555557ce0 - 0x08
0x555555557cd8 <vtable for Derive+8>:	0x0000555555557d38	0x0000555555555404
0x555555557ce8 <vtable for Derive+24>:	0x0000555555555442	0xfffffffffffffff0
dap> -exec x 0x0000555555557d38
0x555555557d38 <typeinfo for Derive>:	0x00007ffff7e6eca8
dap> -exec x 0x0000555555555404
0x555555555404 <Derive::func_a()>:	0xe5894855fa1e0ff3
dap> -exec x 0x0000555555555442
0x555555555442 <Derive::func_b()>:	0xe5894855fa1e0ff3
dap> -exec x/4g 0x555555557d00 - 0x08
0x555555557cf8 <vtable for Derive+40>:	0x0000555555557d38	0x0000555555555480
0x555555557d08 <vtable for Base_b>:	0x0000000000000000	0x0000555555557d70
dap> -exec x 0x0000555555557d38
0x555555557d38 <typeinfo for Derive>:	0x00007ffff7e6eca8
dap> -exec x 0x0000555555555480
0x555555555480 <non-virtual thunk to Derive::func_b()>:	0x10ef8348fa1e0ff3
dap> -exec disas 0x555555555480 
Dump of assembler code for function non-virtual thunk to Derive::func_b():
   0x0000555555555480 <+0>:	endbr64
   0x0000555555555484 <+4>:	sub    rdi,0x10
   0x0000555555555488 <+8>:	jmp    0x555555555442 <Derive::func_b()>
End of assembler dump.
```

                      ┌─────────────────────┐
                      │ class Derive        │            Derive的vtable
    ┌──────────┐      │      Base_a         │
    │Base_a *a ├──────┼──────►  _vptr.Base_a├──────────┐   主虚函数表块
    └──────────┘      │         a = 10      │          │     ┌───────────────────┐
    ┌──────────┐      │      Base_b         │          │     │typeinfo for Derive│
    │Base_b *b ├──────┼──────►  _vptr.Base_b├──────┐   └────►│Derive::func_a()   │
    └──────────┘      │         b = 20      │      │         │Derive::func_b()   │◄────────────────────────────────┐
                      │      d = 40         │      │         └───────────────────┘                                 │
                      └─────────────────────┘      │                                                               │
                                                   │                                                               │
                                                   │       子虚函数表块                                            │
                                                   │          ┌─────────────────────────┐                          │
                                                   │          │typeinfo for Derive      │                          │
                                                   └─────────►│thunk of Derive::func_b()├─┐                        │
                                                              └─────────────────────────┘ │                        │
                                                                                          │                        │
                                                                   thunk代码              ▼                        │
                                                                     ┌─────────────────────────────────────────┐   │
                                                                     │ endbr64                                 │   │
                                                                     │ sub    rdi,0x10                         │   │
                                                                     │ jmp    0x555555555442 <Derive::func_b()>├───┘
                                                                     └─────────────────────────────────────────┘


### 菱形继承(未使用虚继承)

```cc
#include <iostream>
using namespace std;
class Base_a {
    public:
        virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        int a = 10;
};
class Base_b: public Base_a {
    public:
        virtual void func_b() { cout << __PRETTY_FUNCTION__ << endl;}
        long b = 20;
};
class Base_c: public Base_a {
    public:
        virtual void func_c() { cout << __PRETTY_FUNCTION__ << endl;}
        long c = 30;
};
class Derive: public Base_b, public Base_c {
    public:
        virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        virtual void func_b() {cout << __PRETTY_FUNCTION__ << endl;}
        int d = 40;
};
int main() {
    std::cout << "sizeof(Base_a): " << sizeof(Base_a) << std::endl; // 16
    std::cout << "sizeof(Base_b): " << sizeof(Base_b) << std::endl; // 24
    std::cout << "sizeof(Base_c): " << sizeof(Base_c) << std::endl; // 24
    std::cout << "sizeof(Derive): " << sizeof(Derive) << std::endl; // 56
    Derive* d = new Derive();
    std::cout << d << std::endl;      // 0x55555556b6c0
    Base_b* b = d;
    std::cout << b << std::endl;      // 0x55555556b6c0
    Base_c* c =d;
    std::cout << c << std::endl;      // 0x55555556b6d8

    d->func_a();  // virtual void Derive::func_a()
    b->func_b();  // virtual void Derive::func_b()
    c->func_c();  // virtual void Base_c::func_c()

    c->func_a();  // virtual void Derive::func_a()
// Dump of assembler code for function non-virtual thunk to Derive::func_a():
//    0x00005555555554c2 <+0>:	endbr64
//    0x00005555555554c6 <+4>:	sub    rdi,0x18
//    0x00005555555554ca <+8>:	jmp    0x555555555484 <Derive::func_a()>
// End of assembler dump.

    return 0;
}
```

#### gdb分析内存布局
```txt
dap> -exec p *d
$1 = {
  <Base_b> = {
    <Base_a> = {
      _vptr.Base_a = 0x555555557c90 <vtable for Derive+16>,
      a = 10
    }, 
    members of Base_b:
    b = 20
  }, 
  <Base_c> = {
    <Base_a> = {
      _vptr.Base_a = 0x555555557cb0 <vtable for Derive+48>,
      a = 10
    }, 
    members of Base_c:
    c = 30
  }, 
  members of Derive:
  d = 40
}
dap> -exec x/4xg 0x555555557c90 - 0x08
0x555555557c88 <vtable for Derive+8>:	0x0000555555557d18	0x0000555555555484
0x555555557c98 <vtable for Derive+24>:	0x00005555555554cc	0xffffffffffffffe8
dap> -exec x 0x0000555555557d18
0x555555557d18 <typeinfo for Derive>:	0x00007ffff7e6eca8
dap> -exec x 0x0000555555555484
0x555555555484 <Derive::func_a()>:	0xe5894855fa1e0ff3
dap> -exec x 0x00005555555554cc
0x5555555554cc <Derive::func_b()>:	0xe5894855fa1e0ff3
dap> -exec x/4xg 0x555555557cb0
0x555555557cb0 <vtable for Derive+48>:	0x00005555555554c2	0x0000555555555446
0x555555557cc0 <vtable for Base_c>:	0x0000000000000000	0x0000555555557d50
dap> -exec x 0x00005555555554c2
0x5555555554c2 <non-virtual thunk to Derive::func_a()>:	0x18ef8348fa1e0ff3
dap> -exec x 0x0000555555555446
0x555555555446 <Base_c::func_c()>:	0xe5894855fa1e0ff3
dap> -exec disas 0x5555555554c2
Dump of assembler code for function non-virtual thunk to Derive::func_a():
   0x00005555555554c2 <+0>:	endbr64
   0x00005555555554c6 <+4>:	sub    rdi,0x18
   0x00005555555554ca <+8>:	jmp    0x555555555484 <Derive::func_a()>
End of assembler dump.
```

```ascii

                      ┌──────────────────────────┐
                      │  class Derive            │              Derive的虚函数表
  ┌───────────┐       │      Base_b              │                 主表
  │ Derive *d ├───────┼─┐       Base_a           │                  ┌───────────────────┐
  └───────────┘       │ ├────────► _vptr.Base_a  ├───────────────┐  │ typeinfo of Derive│
  ┌───────────┐       │ │          a = 10        │               └──┼►Derive::func_a()  │◄───────────────────────────┐
  │ Base_b *b ├───────┼─┘       b = 20           │                  │ Derive::func_b()  │                            │
  └───────────┘       │      Base_c              │                  └───────────────────┘                            │
  ┌───────────┐       │         Base_a           │                                                                   │
  │ Base_c *c ├───────┼──────────► _vptr.Base_a  ├───────────────┐  子表                                             │
  └───────────┘       │            a = 10        │               │   ┌───────────────────────────┐                   │
                      │         c = 30           │               │   │ typeinfo of Derive        │                   │
                      │      d = 40              │               └───┼►thunk to Derive::func_a() ├─┐                 │
                      │                          │                   │ Base_c::func_c()          │ │                 │
                      └──────────────────────────┘                   └───────────────────────────┘ │                 │
                                                                                                   │                 │
                                                                     thunk代码                     ▼                 │
                                                                      ┌──────────────────────────────────────────┐   │
                                                                      │ endbr64                                  │   │
                                                                      │ sub    rdi,0x18                          │   │
                                                                      │ jmp    0x555555555484 <Derive::func_a()> ├───┘
                                                                      └──────────────────────────────────────────┘

```


### 菱形继承(使用虚继承, 没有虚函数)

```cc
#include <iostream>
using namespace std;
class Base_a {
    public:
        void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        int a = 10;
};
class Base_b: virtual public Base_a {
    public:
        void func_b() { cout << __PRETTY_FUNCTION__ << endl;}
        long b = 20;
};
class Base_c: virtual public Base_a {
    public:
        void func_c() { cout << __PRETTY_FUNCTION__ << endl;}
        long c = 30;
};
class Derive: virtual public Base_b, virtual public Base_c {
    public:
        void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        void func_b() {cout << __PRETTY_FUNCTION__ << endl;}
        int d = 40;
};
int main() {
    std::cout << "sizeof(Base_a): " << sizeof(Base_a) << std::endl; // 16
    std::cout << "sizeof(Base_b): " << sizeof(Base_b) << std::endl; // 32
    std::cout << "sizeof(Base_c): " << sizeof(Base_c) << std::endl; // 32
    std::cout << "sizeof(Derive): " << sizeof(Derive) << std::endl; // 64
    Derive* d = new Derive();
    std::cout << d << std::endl;      // 0x55555556b6c0
    Base_b* b = d;
    std::cout << b << std::endl;      // 0x55555556b6c0
    Base_c* c =d;
    std::cout << c << std::endl;      // 0x55555556b6d8

    d->func_a();  // virtual void Derive::func_a()
    b->func_b();  // virtual void Derive::func_b()
    c->func_c();  // virtual void Base_c::func_c()

    c->func_a();  // virtual void Derive::func_a()

    cout << d->a << endl;
    // 1413:	mov    -0x30(%rbp),%rax
    // 1417:	mov    (%rax),%rax     ; _vptr.Derive
    // 141a:	sub    $0x20,%rax      ; _vptr.Derive - 0x20
    // 141e:	mov    (%rax),%rax     ; 获得偏移值 
    // 1421:	mov    %rax,%rdx
    // 1424:	mov    -0x30(%rbp),%rax ; &Derive
    // 1428:	add    %rdx,%rax       ; &Derive + 0x20 = a的地址
    // 142b:	mov    (%rax),%eax     ; 获得a的值
    // 142d:	mov    %eax,%esi
    // 142f:	lea    0x2c0a(%rip),%rax        # 4040 <std::cout@GLIBCXX_3.4>
    // 1436:	mov    %rax,%rdi
    // 1439:	call   10f0 <std::ostream::operator<<(int)@plt>

    cout << d->b <<endl;
    // 1470:	mov    -0x30(%rbp),%rax
    // 1474:	mov    (%rax),%rax
    // 1477:	sub    $0x18,%rax
    // 147b:	mov    (%rax),%rax
    // 147e:	mov    %rax,%rdx
    // 1481:	mov    -0x30(%rbp),%rax
    // 1485:	add    %rdx,%rax
    // 1488:	mov    0x8(%rax),%rax
    // 148c:	mov    %rax,%rsi
    // 148f:	lea    0x2baa(%rip),%rax        # 4040 <std::cout@GLIBCXX_3.4>
    // 1496:	mov    %rax,%rdi
    // 1499:	call   1110 <std::ostream::operator<<(long)@plt>

    cout << d->c << endl;
    // 14b0:	mov    -0x30(%rbp),%rax
    // 14b4:	mov    (%rax),%rax
    // 14b7:	sub    $0x28,%rax
    // 14bb:	mov    (%rax),%rax
    // 14be:	mov    %rax,%rdx
    // 14c1:	mov    -0x30(%rbp),%rax
    // 14c5:	add    %rdx,%rax
    // 14c8:	mov    0x8(%rax),%rax
    // 14cc:	mov    %rax,%rsi
    // 14cf:	lea    0x2b6a(%rip),%rax        # 4040 <std::cout@GLIBCXX_3.4>
    // 14d6:	mov    %rax,%rdi
    // 14d9:	call   1110 <std::ostream::operator<<(long)@plt>

    cout << d->d << endl;
    // 14f0:	mov    -0x30(%rbp),%rax
    // 14f4:	mov    0x8(%rax),%eax
    // 14f7:	mov    %eax,%esi
    // 14f9:	lea    0x2b40(%rip),%rax        # 4040 <std::cout@GLIBCXX_3.4>
    // 1500:	mov    %rax,%rdi
    // 1503:	call   1100 <std::ostream::operator<<(int)@plt>

    d->func_a();
    // 151a:	mov    -0x30(%rbp),%rax
    // 151e:	mov    %rax,%rdi
    // 1521:	call   1758 <Derive::func_a()>
    d->func_b();
    // 1526:	mov    -0x30(%rbp),%rax
    // 152a:	mov    %rax,%rdi
    // 152d:	call   1796 <Derive::func_b()>

    d->Base_b::func_b();
    // 1532:	mov    -0x30(%rbp),%rax
    // 1536:	mov    (%rax),%rax
    // 1539:	sub    $0x18,%rax
    // 153d:	mov    (%rax),%rax
    // 1540:	mov    %rax,%rdx
    // 1543:	mov    -0x30(%rbp),%rax
    // 1547:	add    %rdx,%rax
    // 154a:	mov    %rax,%rdi
    // 154d:	call   16dc <Base_b::func_b()>

    d->Base_c::func_c(); // 根据_vptr获得偏移量， d + 偏移量 = Derive::Base_c::this
    // 1552:	mov    -0x30(%rbp),%rax
    // 1556:	mov    (%rax),%rax
    // 1559:	sub    $0x28,%rax
    // 155d:	mov    (%rax),%rax
    // 1560:	mov    %rax,%rdx
    // 1563:	mov    -0x30(%rbp),%rax
    // 1567:	add    %rdx,%rax
    // 156a:	mov    %rax,%rdi
    // 156d:	call   171a <Base_c::func_c()>

    Base_a *a = new Base_a();
    cout << a->a <<endl;

    b = new Base_b();
    cout << b->a << endl;
    // 14c7:	mov    -0x28(%rbp),%rax 
    // 14cb:	mov    (%rax),%rax         ; _vptr.Base_b
    // 14ce:	sub    $0x18,%rax          ; _vptr.Base_b - 0x18
    // 14d2:	mov    (%rax),%rax         ; *(_vptr.Base_b - 0x18) = 偏移量
    // 14d5:	mov    %rax,%rdx           
    // 14d8:	mov    -0x28(%rbp),%rax    ; b
    // 14dc:	add    %rdx,%rax           ; b + 偏移量
    // 14df:	mov    (%rax),%eax         ; *(b + 偏移量) = a
    // 14e1:	mov    %eax,%esi
    // 14e3:	lea    0x2b56(%rip),%rax        # 4040 <std::cout@GLIBCXX_3.4>
    // 14ea:	mov    %rax,%rdi
    // 14ed:	call   10f0 <std::ostream::operator<<(int)@plt>


    c = new Base_c();
    cout << c->a << endl;

    return 0;
}
```

#### gdb分析内存布局

```txt
// NOTE : gdb显示的内存布局是错误的，需要结合实际内存值分析
dap> -exec p *d
$1 = {
  <Base_b> = {
    <Base_a> = {
      a = 10
    }, 
    members of Base_b:
    _vptr.Base_b = 0x555555557c40 <vtable for Derive+64>,
    b = 20
  }, 
  <Base_c> = {
    members of Base_c:
    _vptr.Base_c = 0x555555557c58 <VTT for Derive>,
    c = 30
  }, 
  members of Derive:
  _vptr.Derive = 0x555555557c28 <vtable for Derive+40>,
  d = 40
}
dap> -exec p d
$2 = (Derive *) 0x55555556b6c0
dap> -exec x/8xg d
0x55555556b6c0:	0x0000555555557c28	0x0000000000000028
0x55555556b6d0:	0x0000555555557c40	0x0000000000000014
0x55555556b6e0:	0x000000000000000a	0x0000555555557c58
0x55555556b6f0:	0x000000000000001e	0x0000000000000021
dap> -exec set print asm-demangle on
=cmd-param-changed,param="print asm-demangle",value="on"
dap> -exec x 0x555555557c28 - 0x20
0x555555557c08 <vtable for Derive+8>:	0x0000000000000020
dap> p d
-var-create: unable to create variable object
dap> -exec x/xg 0x55555556b6c0 + 0x20
0x55555556b6e0:	0x000000000000000a
dap> 
```

```ascii

                                             ┌► a在Derive中的偏移量
              class Derive              0x20 │             │
                   _vptr.Derive ────────────►│             │
                   d = 40                                  │
                   Base_b                                  │
                       _vptr.Base_b                        │
                       b = 20                              │
                       Base_a                              │
                           a = 10  ◄───────────────────────┘
                    Base_c
                        _vptr.Base_c
                        c= 30


               class Base_b                          ┌► a在Base_b中的偏移量
                   Base_b                        0x18│      │
                       _vptr.Base_b ────────────────►│      │
                       b = 20                               │
                       Base_a                               │
                           a = 10  ◄────────────────────────┘

```
虚继承会根据虚继承顺序依次将基类放到对象尾部（一般的继承，基类在首部）。
如果多个基类有出现多继承时，只有第一个基类会完整继承，确保不会冗余。

当访问虚基类的成员时，需要通过vbptr指针获得vbtable中的偏移量。


### 菱形继承(使用虚继承, 有虚函数)



```cc
#include <iostream>
using namespace std;
class Base_a {
    public:
        virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        int a = 10;
};
class Base_b: virtual public Base_a {
    public:
        virtual void func_b() { cout << __PRETTY_FUNCTION__ << endl;}
        long b = 20;
};
class Base_c: virtual public Base_a {
    public:
        virtual void func_c() { cout << __PRETTY_FUNCTION__ << endl;}
        long c = 30;
};
class Derive: virtual public Base_b, virtual public Base_c {
    public:
        virtual void func_a() {cout << __PRETTY_FUNCTION__ << endl;}
        virtual void func_b() {cout << __PRETTY_FUNCTION__ << endl;}
        int d = 40;
};
int main() {
    std::cout << "sizeof(Base_a): " << sizeof(Base_a) << std::endl; // 16
    std::cout << "sizeof(Base_b): " << sizeof(Base_b) << std::endl; // 32
    std::cout << "sizeof(Base_c): " << sizeof(Base_c) << std::endl; // 32
    std::cout << "sizeof(Derive): " << sizeof(Derive) << std::endl; // 64
    Derive* d = new Derive();
    std::cout << d << std::endl;      // 0x55555556b6c0 
    Base_b* b = d;
    std::cout << b << std::endl;      // 0x55555556b6d0
    Base_c* c =d;
    std::cout << c << std::endl;      // 0x55555556b6f0

    d->func_a();  // virtual void Derive::func_a()
    b->func_b();  // virtual void Derive::func_b()
    c->func_c();  // virtual void Base_c::func_c()

    c->func_a();  // virtual void Derive::func_a()

    cout << d->a << endl;
    cout << d->b <<endl;
    cout << d->c << endl;
    cout << d->d << endl;
    d->func_a();
    d->func_b();

    d->Base_b::func_b();
    d->Base_c::func_c();
    Base_a *a = new Base_a();
    cout << a->a <<endl;

    b = new Base_b();
    cout << b->a << endl;
    c = new Base_c();
    cout << c->a << endl;

    return 0;
}
```

#### gdb分析内存布局
```txt

dap> -exec p *d
$1 = {
  <Base_b> = {
    <Base_a> = {
      _vptr.Base_a = 0x555555557b40 <vtable for Derive+120>,
      a = 10
    }, 
    members of Base_b:
    _vptr.Base_b = 0x555555557b20 <vtable for Derive+88>,
    b = 20
  }, 
  <Base_c> = {
    members of Base_c:
    _vptr.Base_c = 0x555555557b68 <vtable for Derive+160>,
    c = 30
  }, 
  members of Derive:
  _vptr.Derive = 0x555555557af0 <vtable for Derive+40>,
  d = 40
}
```
gcc处理的虚函数和虚基类是公用_vptr指针，和虚函数表

和多继承类似，如果基类指针指向派生类对象调用虚函数时，通过基类_vptr找到thunk函数，再调用派生类虚函数表中的函数，

不同点为虚基类的属性包括this指针都需要通过_vptr获得偏移量，通过偏移量间接获得具体属性值。

之所以通过Derive地址加偏移量的方式获得基类属性和基类this值，是为了解决多继承冗余问题。因为当多继承冲突时，只有首个基类会有全部属性，后续基类不完整，所以编译阶段无法确定真实偏移量。



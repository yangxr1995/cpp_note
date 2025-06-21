# 前言
## 环境

apt install g++ perl make automake libtool

## 高并发网络模型的区别
- select
  - 跨平台
  - 效率低：每次监听文件描述符数组都要从用户空间拷贝到内核空间
  - 遍历整个文件描述符数组
  - 最大可监听的fd数量不超过FD_SETSIZE
- poll
  - 类似select，无FD_SETSIZE限制
- epoll
  - 仅linux支持
  - 内核态红黑树，增删监听对象快速。
  - LT(level triggered) 事件没有处理，一直通知
  - ET(edge triggered) 只通知一次，每当状态变化时，触发一个事件
- iocp
  - 仅windows支持
  - 线程池

### select
设置位图

              ┌───────────────────────────────────┐
              │void FD_CLR(int fd, fd_set *set);  │
              │int  FD_ISSET(int fd, fd_set *set);│
              │void FD_SET(int fd, fd_set *set);  │
              │void FD_ZERO(fd_set *set);         │
              └───────┬───────────────────────────┘
                      │设置位图
        位图          ▼
        ┌───┬───┬───┬───┬───┬───┬───┬───┐
        │ 1 │ 0 │ 1 │ 1 │ 0 │ 0 │ 0 │...│
        └───┴───┴───┴───┴───┴───┴───┴───┘
                   │
                   │
                   │   将位图拷贝到内核态
        select     │   2. 遍历位图，获得监听的fd
                   │      根据fd获得file，调用file->f_op->poll获得文件当前事件。
          拷贝     │   3. 如果没有目标事件，则线程进入 TASK_INTERRUPTIBLE
                   │   4. 当file收到事件时，可以唤醒相关线程 
                   │   5. 线程被唤醒，再次遍历位图，并调用相关poll获得事件
                   │   6. 当获得事件时，返回
                   │
                   ▼
        ┌───┬───┬───┬───┬───┬───┬───┬───┐ 
        │ 1 │ 0 │ 1 │ 1 │ 0 │ 0 │ 0 │...│ 
        └───┴───┴───┴───┴───┴───┴───┴───┘ 
              ▲
              │
              │遍历位图，调用相关file的poll


### epoll


                                                      【1】 创建监听，阻塞等待

                                     epoll_create                                                epoll_ctl
                                            │                                                         │
                                            │1. 创建                                                  │2. 创建监听节点，并加入epoll系统
                                            │                                                         │
                                            ▼                                                         ▼
         进程文件表               ┌──struct eventpoll(epoll句柄)                      
         文件描述符  文件         │  rbr(红黑树根节点)  ◄────────────────红黑树◄──┐             struct epitem(epoll监听节点) ◄──┐
         ...         ...          │  rdlist(就绪队列)                             └──────────────rbn(红黑树节点)                │            
         100         epoll ◄──────┘  wq(等待队列)       ◄────────────────等待队列 ◄─┐            rdllink(就绪队列节点)          │   
         101         socket ◄────┐                                                  └─────────────wqlist(等待队列节点)          │   
                                 │                                                                                              │
                                 │                                                                                              │
                                 │                                                                                              │
                                 │                                                                                              │
                                 │                                                              struct eppoll_entry(桥结构)     │
                                 │                                                              base ───────────────────────────┘
                                 │                                                              wait                       
                                 │                                                               │
                                 │                                                               │
                                 └─── struct socket                                              │
                                      wq(等待队列) ◄────────────────────等待队列◄────────────────┘
                                      接受队列



                                                     【2】 接受数据包，返回事件

                                                                      epoll_wait
                                                                        ▲▲
                                                 4. 将整个就绪队列的事件││
                                                    返回给用户程序      ││
                                                                        ││
                                                                        ││
                                                                        ││
       进程文件表               ┌──struct eventpoll(epoll句柄)                                                                  
       文件描述符  文件         │  rbr(红黑树根节点)  ◄────────────────红黑树◄──┐             struct epitem(epoll监听节点) ◄──┐ 
       ...         ...          │  rdlist(就绪队列)   ◄─────────┐               └──────────────rbn(红黑树节点)                │ 
       100         epoll ◄──────┘  wq(等待队列)                 └──────就绪队列◄───────────────rdllink(就绪队列节点)          │ 
       101         socket ◄────┐                                                               wqlist(等待队列节点)           │ 
                               │                                                                                              │                                                                                                      
                               │                                         ▲▲                                                   │ 
                               │                  3. 将epitem从等待队列  ││                                                   │ 
                               │                     移动到就绪队列      ││                                                   │ 
                               │                     唤醒线程            ││                   struct eppoll_entry(桥结构)     │ 
                               │                                         ││                   base ───────────────────────────┘ 
                               │                                         ││                   wait                              
                               │                                         ││                    │                                
                               │                                         ││                    │                                
                               └─── struct socket                        ││                    │                                
                                    wq(等待队列) ◄────────────────────等待队列◄────────────────┘                                
                                    接受队列                                                                                    
                                      ▲                                                                                         
                                      │             1. 入栈数据，交给对应socket处理，数据缓存到接受队列
                                      │             2. socket唤醒等待队列，执行相关回调函数            
                                   tcp/ip协议栈                                                                                                               
                                      ▲            
                                      │
                                   网卡驱动                                           
                                                
epoll相对poll高效的原理
- 1.poll每次多路io监听都需要设置监听策略，且修改监听策略需要复制所有策略数据，epoll只需要修改单个监听事件，且不需要重复设置，且底层使用红黑树
- 2. 事件发生后，poll方法需要遍历整个监听数据，找到事件发生的文件描述符，而epoll直接使用就绪队列的数据


# libevent 事件状态


                                       ┌─────────────┐
                                       │ invalid ptr │
                                       └──────┬──────┘
                        检查是否初始化        │
                        event_initialized()   │
                              |               │  event_new()
                              |               ▼
     ┌───────────────┐ event_assign()  ┌────────────┐
     │ uninitialized ├────────────────►│ non-pending│◄───────────────────┐
     │               │◄────────────────┤            │                    │
     └───────────────┘ event_free()    └───┬────────┘                    │
                              | event_add()│     ▲                       │                      检查是否挂起
                              | -----------│-----│-----------------------│--------------------- event_pending()
                              |            │     │ event_del()           │ callback done
                                           ▼     │                       │ && non-persist
                                       ┌─────────┴───┐                   │
                                       │ pending     │                   │
                                       └───┬─────────┘                   │
                                           │     ▲                       │
                       event triggered     │     │ callback done         │
                       || event_active()   │     │ && persist            │
                                           ▼     │                       │
                                       ┌─────────┴───┐                   │
                                       │ active      ├───────────────────┘
                                       └─────────────┘
   
- 已初始化(initialized/non-pending) : 调用 event_new()  
- 待决(pending) : 调用event_add()
- 激活(active) : 事件发生
- 持久(persist) : 设置了持久，激活后就变为pending状态，否则为non-pending

# libevent 核心函数

## TCP listener

### evconnlistener_new_bind
分配一个新的 evconnlistener 对象，用于在指定地址上监听传入的 TCP 连接。

- 参数
  - base 将监听器与事件基线关联。
  - cb 在新连接到达时调用的回调函数。如果回调函数为 NULL，则监听器将被视为禁用，直到回调函数被设置。
  - ptr 用户提供的指针，传递给回调函数。
  - flags 任意数量的 LEV_OPT_* 标志。
  - backlog 传递给 listen() 调用以确定可接受连接等待队列的长度。设为 -1 以使用合理默认值。
  - addr 要监听连接的地址。
  - socklen 地址的长度。

```cc
struct evconnlistener *evconnlistener_new_bind(struct event_base *base,
    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
    const struct sockaddr *sa, int socklen);
```

#### flags
- LEV_OPT_LEAVE_SOCKETS_BLOCKING	(1u<<0)
  - 表示我们不应将传入套接字设置为非阻塞模式，再传递给回调函数
- LEV_OPT_CLOSE_ON_FREE		(1u<<1)
  - 表示释放监听器时应关闭底层
- LEV_OPT_CLOSE_ON_EXEC		(1u<<2)
  - 表示我们应尽可能设置关闭时执行标志
- LEV_OPT_REUSEABLE		(1u<<3)
  - 表示我们应禁用在该套接字关闭与再次监听同一端口之间的超时（如果有的话）
- LEV_OPT_THREADSAFE		(1u<<4)
  - 表示监听器应锁定，以便可以安全地从多个线程中同时使用
- LEV_OPT_DISABLED		(1u<<5)
  - 表示监听器应以禁用状态创建。使用 evconnlistener_enable() 后启用它
- LEV_OPT_DEFERRED_ACCEPT		(1u<<6)
  - 表示当数据可用时，监听器应延迟调用 accept()，如果可能的话
- LEV_OPT_REUSEABLE_PORT		(1u<<7)
  - 表示我们要求允许多个服务器(进程或线程) 如果各自设置了该选项，则可绑定到同一端口
- LEV_OPT_BIND_IPV6ONLY		(1u<<8)
  - 表示监听器仅应在 IPv6 套接字上工作

#### evconnlistener_cb

当监听器建立新连接时，我们调用的回调函数。

- 参数
- listener evconnlistener
- fd 新的文件描述符(已连接的fd)
- addr 连接的源地址
- socklen addr 的长度
- user_arg 传递给 evconnlistener_new() 的指针

```cc
typedef void (*evconnlistener_cb)(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *user_arg);
```

## event
### 基本函数
#### event_new
分配并设置一个新的事件结构，用于准备添加。
event_new()函数返回一个可用于未来event_add()和event_del()调用的新事件。
fd和events参数决定了哪些条件将触发事件；
callback和callback_arg参数告诉Libevent当事件激活时应执行的操作。

如果events包含EV_READ、EV_WRITE或EV_READ EV_WRITE，则fd是需要监控的文件描述符或套接字，用于检测就绪读取、就绪写入或两种操作就绪（分别对应）。
如果events包含EV_SIGNAL，则fd是需要等待的信号编号。
如果events不包含这些标志，则事件只能通过超时或通过event_active()手动激活，在这种情况下，fd必须为-1。
events参数中还可以传递EV_PERSIST标志：它使event_add()持续有效，直到调用event_del()为止。

EV_ET标志与EV_READ和EV_WRITE兼容，仅支持某些后端。它告诉Libevent使用边沿触发事件。
EV_TIMEOUT标志在此处无任何效果。

可以多个事件监听相同的文件描述符；但它们要么全部使用边沿触发，要么全部不使用边沿触发。

当事件激活时，事件循环将运行提供的回调函数，并传递三个参数。
第一个参数是提供的fd值。
第二个参数是触发事件的事件位字段：EV_READ、EV_WRITE或EV_SIGNAL。此处EV_TIMEOUT标志表示发生超时，EV_ET标志表示发生边沿触发事件。
第三个参数是您提供的回调参数指针。

参数：
base — 事件应附加的事件基；
fd — 需要监控的文件描述符或信号，或-1；
events — 需要监控的事件：EV_READ、EV_WRITE、EV_SIGNAL、EV_PERSIST、EV_ET的位字段；
callback — 事件发生时调用的回调函数；
callback_arg — 传递给回调函数的参数。

返回值：新分配的event结构体，之后必须使用event_free()释放，或在发生错误时返回NULL。

参见event_free()、event_add()、event_del()、event_assign()。


```cc
event_new
struct event *event_new(struct event_base *base, int fd, short events, event_callback_fn callback,
                        void * callback_arg)
```
   

#### event_add
event_add()函数在由event_assign()或event_new()指定的条件发生时，或在超时时间到达时执行事件'ev'。
若有超时参数为NULL，则不会发生超时，函数仅在匹配事件发生时被调用。

ev参数中的事件必须已经通过event_assign()或event_new()初始化，并且在non-pending前不能用于event_assign()的调用。

若ev参数中的事件已有预定的超时，调用event_add()会将旧的超时替换为新的超时（若tv非NULL）。

参数 
ev 通过event_assign()或event_new()初始化的事件结构体  
timeout 等待事件的最大时间，为NULL时表示无限等待  

返回值 
0表示成功，-1表示发生错误  

参见 event_del(), event_assign(), event_new()  
```cc
int event_add(struct event *ev, const struct timeval *timeout);
```


#### event_del
从监控的事件集合中删除一个事件。

event_del() 函数会取消参数 ev 中的事件。如果该事件已经执行过或从未被添加过，则调用此函数将无任何效果。

参数 ev：要从工作集移除的事件结构体
返回值：成功返回 0，出错返回 -1
参见：event_add()
```cc
int event_del(struct event *);
```

#### event_assign

准备一个全新的、已分配的事件结构体用于添加。

函数event_assign()将事件结构体ev准备用于后续对event_add()和event_del()的调用。与event_new()不同，它不会自行分配内存：它需要你已预先分配了一个struct event结构体，通常是在堆上分配。这样会使你的代码依赖于事件结构体的大小，从而与Libevent的未来版本产生不兼容性。

避免此问题的最简单方法是直接使用event_new()和event_free()。

更复杂一点的方法是使用event_get_struct_event_size()在运行时确定事件结构体所需的大小，以使代码兼容未来版本。

请注意：在active或pending的事件上调用此函数是不安全的。这样做会导致Libevent的内部数据结构被破坏，并引发难以诊断的奇怪错误。你可以使用event_assign()修改现有事件，但仅限于该事件既不处于active也不处于pending！

该函数的参数及所创建事件的行为与event_new()相同。

参数：
- ev：要修改的事件结构体
- base：ev应附加的事件基
- fd：需监控的文件描述符
- events：要监控的事件类型；可为EV_READ和/或EV_WRITE
- callback：事件触发时调用的回调函数
- callback_arg：传递给回调函数的参数

返回值：
- 成功返回0，参数无效返回-1。

参考：event_new(), event_add(), event_del(), event_base_once(), event_get_struct_event_size()

```cc
int event_assign(struct event *, struct event_base *, evutil_socket_t, short, event_callback_fn, void *);
```

#### event_free

释放由 event_new() 返回的 struct event * 指针。

如果事件处于pending或active，此函数会先将其设为non-pending和non-active。
```cc
void event_free(struct event *);
```

#### event_pending
检查特定事件是否处于pending或scheduled状态。

参数 
ev：先前传递给 event_add() 的事件结构体
events：请求的事件类型；可以是 EV_TIMEOUT、EV_READ、EV_WRITE、EV_SIGNAL 中的任意一种
tv：如果此字段不为 NULL 且事件设置了超时，该字段将被设置为保存超时时间的值。

返回值：
如果事件在 'events' 中的任意一种类型上处于pending状态（即已添加），返回 true；
如果事件non-pending(未被添加)，返回 0。

```cc
int event_pending(const struct event *ev, short events, struct timeval *tv);
```

#### event_self_cbarg

返回一个值，用于指定事件本身必须作为回调参数。

函数event_new()接受一个回调参数，该参数将传递给事件的回调函数。若要指定传递给回调函数的参数是event_new()返回的事件，则将event_self_cbarg()的返回值作为event_new()的回调参数传入。

例如：
```cc
struct event *ev = event_new(base, sock, events, callback, event_self_cbarg());
```
为了与event_new()保持一致，可以将此函数的返回值作为event_assign()的回调参数传入——这实现了与直接传递事件相同的效果。

返回一个值，该值将作为传递给event_new()或event_assign()的回调参数。
参见event_new()，event_assign()

```cc
void *event_self_cbarg(void);

```
### 封装函数
#### 信号
##### 宏
```cc
#define evsignal_add(ev, tv)		event_add((ev), (tv))
#define evsignal_assign(ev, b, x, cb, arg)			\
	event_assign((ev), (b), (x), EV_SIGNAL|EV_PERSIST, cb, (arg))
#define evsignal_new(b, x, cb, arg)				\
	event_new((b), (x), EV_SIGNAL|EV_PERSIST, (cb), (arg))
#define evsignal_del(ev)		event_del(ev)
#define evsignal_pending(ev, tv)	event_pending((ev), EV_SIGNAL, (tv))
#define evsignal_initialized(ev)	event_initialized(ev)
```

#### 定时器
##### 宏
```cc
#define evtimer_assign(ev, b, cb, arg) \
	event_assign((ev), (b), -1, 0, (cb), (arg))
#define evtimer_new(b, cb, arg)		event_new((b), -1, 0, (cb), (arg))
#define evtimer_add(ev, tv)		event_add((ev), (tv))
#define evtimer_del(ev)			event_del(ev)
#define evtimer_pending(ev, tv)		event_pending((ev), EV_TIMEOUT, (tv))
#define evtimer_initialized(ev)		event_initialized(ev)
```

##### event_base_init_common_timeout
准备一个事件基，以使用大量具有相同持续时间的超时。

Libevent默认的调度算法针对大量超时时间大致随机分布的情况进行了优化。
但如果你有大量超时时间相同的情况（例如，许多连接均设置了10秒的超时），那么你可以通过告知Libevent来提升其性能。

为此，调用此函数并传入公共持续时间。它将返回指向不同且不透明超时值的指针。（不要依赖其实际内容！）在event_add()中使用此超时值时，Libevent将更高效地调度事件。

（此优化只有在拥有数千或数万相同超时事件时才可能有价值。）
```cc
const struct timeval *event_base_init_common_timeout(struct event_base *base,
    const struct timeval *duration);
```

## 循环
### 运行循环
#### event_base_loop

等待事件变为活动状态，并运行其回调函数。

这是 event_base_dispatch() 的更灵活版本。

默认情况下，此循环会持续运行事件基础结构，直到没有更多待处理或活动的事件，或直到某个调用 event_base_loopbreak() 或 event_base_loopexit()。您可以通过 "flags" 参数覆盖此行为。

- 参数 
  - eb：由 event_base_new() 或 event_base_new_with_config() 返回的事件基础结构
  - flags：任意组合的 EVLOOP_ONCE 和 EVLOOP_NONBLOCK
    - EVLOOP_ONCE : 阻塞直到有一个活动事件，然后在所有活动事件的回调执行完毕后退出
    - EVLOOP_NONBLOCK : 不阻塞，查看当前哪些事件已就绪，运行最高优先级事件的回调，然后退出。
    - EVLOOP_NO_EXIT_ON_EMPTY : 不因没有pending事件而退出循环。改为持续运行，直到 event_base_loopexit() 或 event_base_loopbreak() 让我们停止。
- 返回值：
  - 成功返回 0，
  - 出错返回 -1，
  - 因无待处理/活动事件而退出返回 1。


```cc
int event_base_loop(struct event_base *, int);
```

#### event_base_dispatch
事件派发循环

该循环会持续运行事件基，直到没有更多待处理或活跃的事件，或者直到某个调用 event_base_loopbreak() 或 event_base_loopexit()。

base 由 event_base_new() 或 event_base_new_with_config() 返回的 event_base 结构

返回 0 表示成功，-1 表示发生错误，或 1 表示因没有pending或active的事件而退出。

```cc
int event_base_dispatch(struct event_base *);

while (有任意事件注册，或EVLOOP_NO_EXIT_ON_EMPTY被设置) {
    if (EVLOOP_NONBLOCK 被设置，或任意事件就绪)
        如果任意事件被触发，标记他为active
    else
        等待直到至少一个事件被触发，标记他为active

    for () {
        根据优先级顺序，运行active的事件
    }

    if (EVLOOP_ONCE 被设置 或者 EVLOOP_NONBLOCK 被设置)
        break;
}
```

### 停止循环

#### event_base_loopexit

在指定时间后退出事件循环

在给定计时器到期后，下一个 event_base_loop() 迭代将正常完成（处理所有排队事件），然后退出，不再阻塞等待新事件。
后续对 event_base_loop() 的调用将恢复正常执行。

参数 
- eb：由 event_init() 返回的 event_base 结构
- tv：循环应在该时间后终止，或设为 NULL 以在运行完所有当前活动事件后退出

返回值：
- 成功返回 0
- 出错返回 -1

```cc
int event_base_loopexit(struct event_base *, const struct timeval *);
```

#### event_base_loopbreak

立即中止当前活动的 event_base_loop()。

event_base_loop() 会在下一个事件完成之后中止循环；
event_base_loopbreak() 通常由该事件的回调函数调用。
这种行为与 "break;" 语句类似。

后续对 event_base_loop() 的调用将正常执行。

参数 
- eb：由 event_init() 返回的 event_base 结构体

返回值
- 成功返回 0
- 发生错误返回 -1

```cc
int event_base_loopbreak(struct event_base *);
```

## bufferevent

缓冲事件 I/O 缓冲区

Libevent 在常规事件回调之上提供了缓冲 I/O 抽象，这种抽象称为缓冲事件（bufferevent）。
缓冲事件提供了输入和输出缓冲区，这些缓冲区会自动填充和释放。
使用缓冲事件的用户不再直接处理 I/O，而是从输入缓冲区读取数据，向输出缓冲区写入数据。

一旦通过 bufferevent_socket_new() 初始化，缓冲事件结构体可以通过 bufferevent_enable() 和 bufferevent_disable() 重复使用。
而不是直接对套接字进行读写，应调用 bufferevent_read() 和 bufferevent_write()。

当读取已启用时，缓冲事件会尝试从文件描述符读取数据，并调用读取回调函数。
在输出缓冲区的水位低于写入低水位标记（默认为 0）时，会执行写入回调函数。

### 基本概念
#### 缓冲区
- **输入缓冲区**：存放从底层网络读取到的数据
- **输出缓冲区**：存放待写入底层网络的数据
- **evbuffer**：libevent提供的链式缓冲区实现，支持高效添加/移除数据

#### 回调和水位

##### 输入缓存

       输入缓存             
      ┌────────┐            
      │        │            
     ─┼────────┼──  高水位线
      │        │            
      │        │            
     ─┼────────┼──  低水位线
      │        │            
      └────────┘            

- 高水位线: 决定是否继续缓存数据。
  - 高于高水位: 不再缓存数据
  - 低于高水位: 尽可能缓存数据
- 低水位线: 决定是否调用读取回调函数。
  - 高于低水位: 调用读取回调
  - 低于低水位: 不调用读取回调

下面是个实际的例子：

    1) 当水位低于高水位线时，只要fd有可读数据，bufferevent就会读取并缓存。
    2) 当前水位超过高水位线时，即使fd还有可读数据，bufferevent也会停止读取

        输入缓存                                          输入缓存             
       ┌────────┐                                        ┌────────┐            
       │        │                                        │--------│    当前水位
      ─┼────────┼──  高水位线   读取fd,并缓存数据       ─┼────────┼──  高水位线    调用读取回调，消耗输入缓存的数据
       │        │               ──────────────────►      │        │               ──────────────────────────────────►
       │        │                                        │        │            
      ─┼────────┼──  低水位线                           ─┼────────┼──  低水位线
       │--------│    当前水位                            │        │            
       └────────┘                                        └────────┘            
                                                                               
    3) 当前水位高于低水位线时，bufferevent会调用读取回调函数
    4) 执行读取回调函数，从输入缓存中取出数据
    5) 读取回调函数将缓冲区的所有数据消耗完了
    6) 当前水位处于高水位线以下，bufferevent继续缓存fd的数据

        输入缓存                                          输入缓存
       ┌────────┐                                        ┌────────┐            
       │        │                                        │        │            
      ─┼────────┼──  高水位线    读取fd,并缓存数据      ─┼────────┼──  高水位线
       │        │                ──────────────────►     │        │            
       │        │                                        │        │            
      ─┼────────┼──  低水位线                           ─┼────────┼──  低水位线
       │        │                                        │--------│    当前水位
       └────────┘                                        └────────┘            
   
    7) 输入缓存虽然有数据，但当前水位低于低水位线，bufferevent不会调用读取回调
   
##### 输出缓存
- 高水位: 没有实际作用
- 低水位: 低于低水位时，调用写入回调。

     1) 当前水位低于低水位线时，bufferevent调用写入回调
     2) 写入回调向输出缓存写入数据

        输出缓存                                          输出缓存             
       ┌────────┐                                        ┌────────┐            
       │        │                                        │--------│    当前水位
      ─┼────────┼──  高水位线    读取fd,并缓存数据      ─┼────────┼──  高水位线
       │        │                ──────────────────►     │        │            
       │        │                                        │        │            
      ─┼────────┼──  低水位线                           ─┼────────┼──  低水位线
       │--------│    当前水位                            │        │            
       └────────┘                                        └────────┘            


#### 事件回调
- BEV_EVENT_READING	 读取时遇到错误 
- BEV_EVENT_WRITING	 写入时遇到错误 
- BEV_EVENT_EOF		 文件末尾到达
- BEV_EVENT_ERROR		 遇到不可恢复的错误
- BEV_EVENT_TIMEOUT	 用户指定的超时时间到达
- BEV_EVENT_CONNECTED	 连接操作完成

### 函数 
#### bufferevent_socket_new
创建一个基于现有套接字的缓冲事件（bufferevent）。

- 参数 
  - base 将新缓冲事件与之关联的事件基（event base）。
  - fd 从其中读取和写入数据的文件描述符。
    - 该文件描述符不允许是管道（pipe(2)）。
    - 只要之后使用 bufferevent_setfd 或 bufferevent_socket_connect 设置 fd，
    - 将其设为 -1 是安全的。(libevent内部创建tcp socket)
  - options 零个或多个 BEV_OPT_* 标志
    - BEV_OPT_CLOSE_ON_FREE
      - 如果设置，当此bufferevent被释放时，我们将关闭底层文件描述符bufferevent无论什么情况
    - BEV_OPT_THREADSAFE
      - 如果设置，并且启用了线程，此bufferevent的操作由锁保护
    - BEV_OPT_DEFER_CALLBACKS
      - 如果设置，回调将在事件循环中延迟执行(优先级设为最低)
    - BEV_OPT_UNLOCK_CALLBACKS
      - 如果设置，回调将在不持有bufferevent锁的情况下执行。此选项当前要求同时设置BEV_OPT_DEFER_CALLBACKS；未来版本的Libevent可能移除此要求
      - 主要解决回调函数中调用libevent函数可能导致死锁的问题。
- 返回
  - 一个新分配的 bufferevent 结构体指针，若发生错误则返回 NULL
```cc
struct bufferevent *bufferevent_socket_new(struct event_base *base, evutil_socket_t fd, int options);
```

#### bufferevent_enable
- 参数 
  - bufev 要启用的缓冲事件
  - event 任意组合的 EV_READ、EV_WRITE。
- 返回值
  - 成功返回 0，出错返回 -1
```cc
int bufferevent_enable(struct bufferevent *bufev, short event);
int bufferevent_disable(struct bufferevent *bufev, short event);
short bufferevent_get_enabled(struct bufferevent *bufev);
```

#### bufferevent_setcb

更改缓冲区事件的回调函数。

- 参数 
  - bufev：需要更改回调函数的缓冲区事件对象  
  - readcb：有数据可读时调用的回调函数，若不需要回调则设为NULL  
  - writecb：文件描述符准备好写入时调用的回调函数，若不需要回调则设为NULL  
  - eventcb：文件描述符发生事件时调用的回调函数  
  - cbarg：将传递给各个回调函数（readcb、writecb和errorcb）的参数  

```cc
void bufferevent_setcb(struct bufferevent *bufev,
    bufferevent_data_cb readcb, bufferevent_data_cb writecb,
    bufferevent_event_cb eventcb, void *cbarg);

```


##### bufferevent_data_cb
用于 bufferevent 的读或写回调函数。
读回调函数在输入缓冲区中有新数据到达且可读数据量超过低水位线（默认值为 0）时触发。
写回调函数在写缓冲区已耗尽或降至低水位线以下时触发。

- 参数 
  - bev 触发回调的 bufferevent
  - ctx 该 bufferevent 的用户指定上下文

```cc
typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
```


##### bufferevent_event_cb

一个缓冲事件（bufferevent）的事件错误回调。

如果遇到 EOF 条件或其它不可恢复的错误，该事件回调将被触发。

对于具有延迟回调的缓冲事件，此参数是自上次回调调用以来该缓冲事件上发生的所有错误的按位或运算结果。

- 参数
  - bev 触发错误条件的缓冲事件
  - what 一组标志：BEV_EVENT_READING 或 BEV_EVENT_WRITING 表示错误发生在读取或写入路径，并包含以下标志之一：
    - BEV_EVENT_EOF、BEV_EVENT_ERROR、BEV_EVENT_TIMEOUT、BEV_EVENT_CONNECTED。
  - ctx 为该缓冲事件指定的用户上下文
```cc
typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short what, void *ctx);
```

#### bufferevent_read

从bufferevent缓冲区读取数据。

bufferevent_read()函数用于从输入缓冲区中读取数据。

- 参数
  - bufev 要从中读取的bufferevent
  - data 指向将存储数据的缓冲区的指针
  - size 数据缓冲区的大小，以字节为单位

- 返回值
  - 读取的数据量，以字节为单位。
```cc
size_t bufferevent_read(struct bufferevent *bufev, void *data, size_t size);
```

#### bufferevent_write

将数据写入缓冲事件缓冲区。

bufferevent_write() 函数可用于将数据写入文件描述符。数据会被追加到输出缓冲区，并在描述符可写时自动写入。

- 参数
  - bufev：要写入的缓冲事件
  - data：要写入的数据指针
  - size：数据长度（以字节为单位）

- 返回 
  - 0 表示成功，返回 -1 表示发生错误

```cc
int bufferevent_write(struct bufferevent *bufev,
    const void *data, size_t size);
```

#### bufferevent_setwatermark

设置读取和写入事件的水位线。

在输入时，bufferevent 除非缓冲区中至少有低水位线的数据量，否则不会调用用户的读取回调函数。
如果读取缓冲区超过高水位线，bufferevent 将停止从网络读取数据。
但请注意，bufferevent 输入读取缓冲区可能会超出高水位线限制（典型的例子是 openssl bufferevent），因此你不应该依赖于此。

在输出时，每当缓冲的数据量低于低水位线时，会调用用户的写入回调函数。
写入此 bufev 的过滤器会尝试避免向此缓冲区写入超过高水位线允许的字节数，除非在刷新时。

参数
bufev 要修改的 bufferevent
events EV_READ、EV_WRITE 或两者
lowmark 设置的低水位线
highmark 设置的高水位线

```cc
void bufferevent_setwatermark(struct bufferevent *bufev, short events,
    size_t lowmark, size_t highmark);
```

#### bufferevent_set_timeouts

设置 bufferevent 的读写超时。
bufferevent 的超时会在自上次成功读取或写入操作以来，经过指定时间后首次触发，此时 bufferevent 正在尝试进行读取或写入操作。
（换句话说，如果读取或写入被禁用，或者由于没有数据可写、带宽不足等原因导致读写操作被暂停，超时不会生效。只有当实际愿意进行读取或写入时，超时才会生效。）
调用 bufferevent_enable 或为已设置超时等待的 bufferevent 设置超时，会重置其超时。
如果超时发生，对应的读或写操作（EV_READ 或 EV_WRITE）将被禁用，直到再次启用。bufferevent 的事件回调函数将被调用，并传入 BEV_EVENT_TIMEOUT | BEV_EVENT_READING 或 BEV_EVENT_TIMEOUT | BEV_EVENT_WRITING。

- 参数：
  - bufev：要修改的 bufferevent
  - timeout_read：读取超时，或 NULL
  - timeout_write：写入超时，或 NULL

```cc
int bufferevent_set_timeouts(struct bufferevent *bufev,
    const struct timeval *timeout_read, const struct timeval *timeout_write);
```

#### bufferevent_socket_connect

使用基于套接字的bufferevent发起一次connect()尝试。
当连接成功时，事件回调函数将被调用，并设置BEV_EVENT_CONNECTED标志。
如果bufferevent尚未绑定套接字，我们在此处分配一个新的套接字，并在开始前将其设置为非阻塞模式。
如果没有提供地址，我们假设套接字已经处于连接状态，并配置bufferevent，使其在连接完成后触发BEV_EVENT_CONNECTED事件。

- 参数 
  - bufev：一个通过bufferevent_socket_new()分配的现有bufferevent。
  - addr：我们应连接的地址。
  - socklen：地址的长度。
- 返回 
  - 0 表示成功，-1 表示失败。

```cc
int bufferevent_socket_connect(struct bufferevent *, const struct sockaddr *, int);
```

#### bufferevent_free
释放与bufferevent结构体关联的存储空间。
如果存在待写数据，这些数据很可能在bufferevent被释放前不会被刷新。(可能导致数据处理不完整)
- 参数 
  - bufev 要释放的bufferevent结构体。
```cc
void bufferevent_free(struct bufferevent *bufev);
```

#### bufferevent_trigger

这两个接口的区别主要在于触发的回调类型和触发条件：

1. `bufferevent_trigger()`：
- 专门触发**数据相关**的回调（读回调或写回调）
- 受水位标记（watermarks）控制（除非指定`BEV_TRIG_IGNORE_WATERMARKS`）
- `iotype`参数指定触发读回调(EV_READ)或写回调(EV_WRITE)

2. `bufferevent_trigger_event()`：
- 专门触发**事件相关**的回调（错误/连接状态变化等）
- 不受水位标记控制
- `what`参数传递的是事件标志（如BEV_EVENT_ERROR等）

两者都支持通过`BEV_OPT_DEFER_CALLBACKS`选项延迟回调执行。

```cc
void bufferevent_trigger(struct bufferevent *bufev, short iotype,
    int options);
void bufferevent_trigger_event(struct bufferevent *bufev, short what,
    int options);
```

### bufferevent filter

#### bufferevent_filter_new
在现有bufferevent之上分配一个新的带过滤功能的bufferevent。

- 参数
  - underlying：底层的bufferevent。
  - input_filter：应用于从底层bufferevent读取数据的过滤器。
  - output_filter：应用于写入底层bufferevent的数据的过滤器。
  - options：bufferevent选项的位字段。
  - free_context：当该bufferevent被释放时用于释放过滤器上下文的函数。
  - ctx：传递给过滤函数的上下文指针。

```cc
struct bufferevent *
bufferevent_filter_new(struct bufferevent *underlying,
		       bufferevent_filter_cb input_filter,
		       bufferevent_filter_cb output_filter,
		       int options,
		       void (*free_context)(void *),
		       void *ctx);


```



##### bufferevent_filter_result
一个用于实现bufferevent过滤器的回调函数。

- 参数
  - src 从其中获取数据的evbuffer。
  - dst 添加数据的目标evbuffer。
  - limit 建议写入dst的最大字节数。过滤器可能忽略此值，但这样做会导致超过dst关联的高水位标记。-1表示"无限制"。
  - mode 表示应以何种方式写入数据：
    - （BEV_NORMAL）尽可能方便地写入数据，
    - （BEV_FLUSH）尽可能多地写入数据，
    - （BEV_FINISH）尽可能多地写入数据，可能包含流结束标记。
  - ctx 用户提供的指针。
- 返回 
  - BEV_OK 表示已写入部分数据；
  - BEV_NEED_MORE 表示在获取到输入前无法生成更多输出；
  - BEV_ERROR 表示错误。

```cc
typedef enum bufferevent_filter_result (*bufferevent_filter_cb)(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx);
```

#### evbuffer_remove
从evbuffer中读取数据并排干读取的字节。

如果请求的字节数多于evbuffer中实际可用的字节数，我们只会提取实际可用的字节数。

- 参数 
  - buf：要读取的evbuffer
  - data：用于存储结果的目标缓冲区
  - datlen：目标缓冲区的最大尺寸
- 返回值
  - 读取的字节数，若无法排干缓冲区则返回-1。

```cc
int evbuffer_remove(struct evbuffer *buf, void *data, size_t datlen);
```

#### evbuffer_add
将数据追加到evbuffer的末尾。  

- 参数 
  - buf：要追加数据的 evbuffer  
  - data：指向数据缓冲区起始位置的指针  
  - datlen：从数据缓冲区复制的字节数  
- 返回 
  - 0 表示成功，-1 表示失败。  

```cc
int evbuffer_add(struct evbuffer *buf, const void *data, size_t datlen);
```

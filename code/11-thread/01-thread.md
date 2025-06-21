# 示例1

```cc
#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

void do_work()
{
    cout << "sub thread start" << endl;
    for (int i = 0; i < 10; i++) {
        cout << "work " << i << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "sub thread end" << endl;
}

int main (int argc, char *argv[]) {

    // 创建子线程
    thread th(do_work);

    cout << "main thread join begin" << endl;
    // 等待子线程退出，并回收资源
    th.join();
    cout << "main thread join end" << endl;
    
    return 0;
}
```

# thread对象生命周期和线程等待和分离

```cc
#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

bool is_exit = false;

void do_work(int a)
{
    cout << "sub thread start" << endl;
    for (int i = 0; i < 3 && is_exit == false; i++) {
        cout << "work " << i << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cout << "sub thread end" << endl;
}

int main (int argc, char *argv[]) {

    // 情形1: 主线程和子线程关联，主线程退出，子线程继续运行，导致段错误
    {
        thread th(do_work, 1);
    }

    // 情形2: 主线程和子线程使用join同步退出
    //        主线程需要有方法通知子线程退出，否则主线程会长期阻塞
    {
        thread th(do_work, 1);
        is_exit = true;
        th.join();
    }

    // 情形3: 主线程和子线程detach
    //        linux : 主线程退出后，子线程自动退出
    //        windows : 主线程退出后，子线程可以作为后台进程继续执行
    {
        thread th(do_work, 1);
        th.detach();
    }
   
    return 0;
}
```

# lock
## 锁
### mutex
互斥锁 lock / unlock

互斥锁的坑
```cc
// 以下代码会导致其他线程无法获得锁
for (;;) {
    mux.lock();
    this_thread::sleep_for(1s);
    mux.unlock();
}

// 需要主动挂起
for (;;) {
    mux.lock();
    this_thread::sleep_for(1s);
    mux.unlock();
    this_thread::sleep_for(1ms);
}
```

### timed_mutex
超时锁，可以记录锁获取的情况，多次超时，可以记录日志，获取错误情况（如死锁）
```cc
timed_mutex tmux;
for(;;) {
    if (!tmux.try_lock_for(milliseconds(1000))) {
        cout << i << "[try_to_lock] timeout" << endl;
        continue;
    }
    this_thread::sleep_for(1s);
    tmux.unlock();
    this_thread::sleep_for(1ms);
}
```

### recursive_mutex recursive_timed_mutex
递归锁
- 同一个线程的同一把锁可以锁多次，避免一些不必要的死锁
- 组合业务，用到同一个锁
```cc
static recursive_mutex rmux;

void task1() {
    rmux.lock();
    cout << "task 1" << endl;
    rmux.unlock();
}

void task2() {
    rmux.lock();
    cout << "task 2" << endl;
    rmux.unlock();
}

void thread() {
    rmux.lock();
    task1();
    task2();
    rmux.unlock();
}
```

### shared_mutex
c++14 支持共享超时互斥锁 shared_timed_mutex
c++17 支持共享互斥锁 shared_mutex
如果只有写时需要互斥，读时不需要
```cc
shared_mutex smux;
void thread_reader() {
    smux.shared_lock();
    // 读操作
    smux.shared_unlock();
    this_thread::sleep_for(1ms);
}

void thread_writer() {
    smux.shared_lock();
    // 读操作
    smux.shared_unlock();
    smux.lock();
    // 写操作
    smux.unlock();
    this_thread::sleep_for(1ms);
}

```

## RAII封装器
RAII(资源申请及初始化): 使用局部对象的自动构造和析构来自动管理资源，
构造函数里进行lock，析构时unlock

### lock_guard  c++11
- 严格基于作用域的互斥体所有权包装器
- adopt_lock ，假设调用方已经拥有互斥的所有权
- 通过 {} 控制锁的临界区
```cc
static mutex mux;

void thread()
{
    mux.lock();
    {
        lock_guard<mutex> lock(mux, adopt_lock); // adopt_lock: 不会再次锁
        this_thread::sleep_for(1s);
        // 释放锁
    }
    {
        lock_guard<mutex> lock(mux); // 锁
        this_thread::sleep_for(1s);  // 释放锁
    }
}
```

### unique_lock c++11

- unique_lock 实现可移动的互斥体所有权包装器
```cc
unique_lock& operator=(const unique_lock&) = delete;
unique_lock& operator=(unique_lock&& __u) noexcept
{
    if(_M_owns) 
        unlock();

    unique_lock(std::move(__u)).swap(*this);

    __u._M_device = 0;
    __u._M_owns = false;

    return *this;
}
```
- 支持临时释放锁
```cc
mutex mux;
{
    unique_lock<mutex> lock(mux);
    lock.unlock(); // 临时释放锁
    lock.lock();
}
```
- 支持 adopt_lock (已经拥有锁，不加锁，出栈时释放锁)
```cc
{
    mux.lock();
    {
        unique_lock<mutex> lock(mux, adopt_lock); // 已经拥有锁
    }
}
```
- 支持 defer_lock (延后拥有锁，不加锁，出栈时不释放)
```cc
{
    {
        unique_lock<mutex> lock(mux, defer_lock); // 绑定锁对象，但不上锁
        lock.lock(); // 加锁，
        // 释放锁
    }
}
```
- 支持 try_to_lock 尝试获得锁，获取失败时通过 owns_lock() 判断
```cc
{
    for (;;) {
        unique_lock<mutex> lock(mux, try_to_lock); // 尝试加锁
        if (lock.owns_lock() == false) // 失败
            continue;
    }
}
```
### shared_lock c++14
为了实现mux.shared_lock而增加
```cc
static shared_timed_mutex tmux;
{
    shared_lock<shared_timed_mutex> lock(tmux); // 调用tmux.shared_lock()
    // 读操作
    // 释放 tmux.shared_unlock()
}
{
    unique_lock<shared_timed_mutex> lock(tmux); // 调用tmux.lock()
    // 写操作
    // 释放 tmux.unlock()
}
```
### scoped_lock c++17
用于多锁避免死锁的RAII封装器
```cc
// 当多个锁时，由于获得锁的顺序不一致会导致死锁
static mutex mux1;
static mutex mux2;
void thread1() {
    mux1.lock();
    mux2.lock();
    // ...
    mux2.unlock();
    mux2.unlock();
}
void thread2() {
    mux2.lock();
    mux1.lock();
    // ...
    mux1.unlock();
    mux2.unlock();
}
```
解决方法 c++11, 使用lock() 同时获得多个锁
```cc
void thread1() {
    lock(mux1, mux2);
    // ...
    mux2.unlock();
    mux2.unlock();
}
void thread2() {
    lock(mux1, mux2);
    // ...
    mux1.unlock();
    mux2.unlock();
}
```
c++17的方法
```cc

{
    mutex mux1, mux2;
    // 两种写法
    scoped_lock lock(mux1, mux2);
    // scoped_lock<mutex, mutex> lock(mux1, mux2);
    // 
    // 自动释放多个锁
}
```

# 条件变量

```cc
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
using namespace std;

mutex mux;
list<string> msg;
condition_variable cv;

void ReadThread(int i)
{
    for (;;) {
        unique_lock<mutex> lock(mux);
        cv.wait(lock); // 完成以下原子操作：
                       // 1. 若已经获得了锁则释放锁
                       // 2. 睡眠在cv上, 直到被唤醒
                       // 3. 唤醒后尝试获得锁，如果失败，则再次睡眠
                       // 4. 获得锁成功则返回
        while (!msg.empty()) {
            string n = msg.front();
            msg.pop_front();
            cout << "thread " << i << " read " << n << endl;
        }
        lock.unlock(); // 可以不解锁
    }
}

void WriteThread()
{
    for (int i = 0; ; ++i) {
        stringstream s;
        s << "msg " << i;
        unique_lock<mutex> lock(mux);
        msg.push_back(s.str());
        lock.unlock();
        cv.notify_one();
        this_thread::sleep_for(1s);
    }
}

int main (int argc, char *argv[]) {
    
    thread th(WriteThread);
    th.detach();
    for (int i = 0; i < 5; ++i) {
        thread th(ReadThread, i);
        th.detach();
    }
    getchar();

    return 0;
}
```
# 线程异步
## promise 和 future
promise 和 future 它们为线程间通信提供了一种机制，允许一个线程传递结果或异常给另一个线程。

### Promise
- **作用**：生产者线程通过`promise`对象存储值或异常。
- 关键方法：
  - `set_value()`：设置共享状态的值。
  - `set_exception()`：设置异常。

### Future
- **作用**：消费者线程通过`future`对象获取`promise`设置的值或异常。
- 关键方法：
  - `get()`：阻塞直到值可用，并返回值。
  - `wait()`：等待结果但不获取。

### 示例代码
```cpp
#include <future>
#include <iostream>
#include <thread>
using namespace std;

void TestPromise(promise<string> p)
{
    cout << "TestPromise begin" << endl;
    this_thread::sleep_for(3s);
    p.set_value("1111");
    this_thread::sleep_for(3s);
    cout << "TestPromise end" << endl;
}

int main (int argc, char *argv[]) {
    promise<string> p;
    future f = p.get_future();
    thread th(TestPromise, std::move(p));
    string ret = f.get();
    cout << "future : " << ret << endl;
    th.join();
    
    return 0;
}
```

**特点**：
- 一对一关系：一个`promise`对应一个`future`。
- 不可复制但可移动。

## packaged_task
packaged_task 将函数包装为对象，用于异步调用，将函数调用和函数返回拆分成两步。
和bind的区别，可用于异步调用，函数访问和获取返回值分开调用.


## packaged_task
C++中的`packaged_task`是一个模板类，用于将可调用对象（如函数、lambda表达式）与一个`future`关联起来，允许异步获取该调用的结果。它属于`<future>`头文件，适合管理一次性异步任务的执行和结果获取。

### 核心特性：
1. **封装可调用对象**  
   将任务包装为可调用对象，存储其返回值或异常。
2. **返回future**  
   通过`get_future()`方法获取与任务结果关联的`future`。
3. **手动触发执行**  
   任务需通过调用`operator()`或移动给线程来启动。


```cc
#include <future>
#include <iostream>
#include <thread>
#include <utility>
using namespace std;

string TestPack(int id) {
    cout << "TestPack begin " << id << endl;
    this_thread::sleep_for(2s);
    cout << "TestPack end " << id << endl;
    return "TestPack return";
}

int main (int argc, char *argv[]) {

    packaged_task<string (int)> task(TestPack);
    future result = task.get_future();
    // task(100);
    thread th(std::move(task), 100); // 移动任务到线程

    cout << "wait future" << endl;
    string ret = result.get();
    cout << "futu result : " << ret << endl;

    th.join();
    
    return 0;
}
```

### 典型用途：
- 需要显式控制任务执行时机时（区别于`async`的自动启动）。
- 与线程池或任务队列配合，延迟执行任务并获取结果。

### 注意事项：
- 一个`packaged_task`只能被调用一次，第二次调用会抛出异常。
- 若任务未被调用，关联的`future`会一直阻塞在`get()`。

## async
- 异步运行一个函数，并返回保有其结果的`std::future`
- `launch::deferred` 延迟执行，在调用`wait`和`get`时，调用函数代码
- `launch::async` 创建线程（默认）
- 返回的线程函数的返回值类型的`std::future<int>` (`std::future<线程函数的返回值类型>`)
- `future.get()`获取结果，会阻塞等待

```cc
#include <future>
#include <iostream>
#include <thread>
using namespace std;

string TestAsync(int id) {
    cout << id << " TestAsync begin " << this_thread::get_id() << endl;
    this_thread::sleep_for(2s);
    return "TestAsync return";
}

int main (int argc, char *argv[]) {
    {
        cout << "main thread " << this_thread::get_id() << endl; 
        future<string> future = async(launch::deferred, TestAsync, 100);
        cout << "future get : " << endl;
        string ret = future.get(); // 延迟执行，所以在get的时候才调用TestAsync
        cout << "ret : " << ret << endl;
    }
    {
        cout <<"---------------" << endl;
        cout << "main thread " << this_thread::get_id() << endl; 
        future<string> future = async(launch::async, TestAsync, 100); // 异步执行TestAsync
        this_thread::sleep_for(1s);
        cout << "future get : " << endl;
        string ret = future.get();
        cout << "ret : " << ret << endl;
    }
    
    return 0;
}
```

# 多核运算 for_each
```cc
    vector<string> s = {"111", "222", "333"};
    for_each(execution::par, 
            s.begin(), s.end(), 
            [](string &s) {
            cout << this_thread::get_id() <<" " ;
            cout << s << endl;
            });
```

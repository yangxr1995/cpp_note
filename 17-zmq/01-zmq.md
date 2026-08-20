# 套接字和模式
## 套接字

### 一、ZeroMQ 套接字 ≠ 传统 BSD 套接字

- **根本区别**：
  - 传统套接字：面向连接/字节流，需手动管理消息边界、重连、多路复用。
  - ZeroMQ 套接字：**面向完整消息**，自动处理异步 I/O、重连、多对多连接聚合。
- ZeroMQ 套接字是**消息队列的端点**，其行为由 **类型决定语义** ，而非底层协议。

---

### 二、五大核心通信模式（Built-in Messaging Patterns）

ZeroMQ 将分布式通信抽象为几种**硬编码的、经过验证的模式**，每种模式由一对兼容的套接字类型实现：

| 模式 | 套接字对 | 用途 | 特点 |
|------|--------|------|------|
| **请求-应答**（Request-Reply） | `REQ` ↔ `REP` | RPC、任务调度 | 同步、严格交替收发 |
| **发布-订阅**（Publish-Subscribe） | `PUB` ↔ `SUB` | 数据广播、事件通知 | 单向、支持主题过滤 |
| **管道**（Pipeline / Push-Pull） | `PUSH` ↔ `PULL` | 并行任务分发与收集 | 扇出→扇入，负载均衡 |
| **独占对**（Exclusive Pair） | `PAIR` ↔ `PAIR` | 线程间通信 | 仅限同一进程内使用 |
| **高级路由基础** | `DEALER` / `ROUTER` | 构建代理、异步服务 | 支持多跳、身份路由（为后续章节铺垫） |

> 📌 书中强调：**必须理解这些模式的语义，而不是仅仅调用 API**。例如，`PUB` 只能发送，`SUB` 默认过滤所有消息（需显式订阅）。

---

### 三、套接字生命周期与基本操作

1. **创建**：`zmq_socket(context, type)`
2. **配置**（可选）：如设置高水位（HWM）、订阅主题、 linger 时间等
3. **绑定或连接**：
   - `zmq_bind()`：通常用于稳定节点（如服务端）
   - `zmq_connect()`：通常用于动态节点（如客户端）
   - **顺序无关**：先启动哪一方均可，ZeroMQ 自动恢复连接
4. **发送/接收消息**：`zmq_msg_send()` / `zmq_msg_recv()`
5. **关闭**：`zmq_close()` + `zmq_ctx_destroy()`

---

#### 示例
##### ✅ 示例 1：PUB/SUB 模式（广播日志）

###### 📡 发布者（Publisher）—— 通常 `bind()`

```c
// pub.c
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    // 1. 创建上下文
    void *context = zmq_ctx_new();

    // 2. 创建 PUB 套接字
    void *publisher = zmq_socket(context, ZMQ_PUB);

    // 3. 【可选】配置：设置高水位（默认可能丢消息）
    int hwm = 1000;
    zmq_setsockopt(publisher, ZMQ_SNDHWM, &hwm, sizeof(hwm));

    // 4. 绑定到端口（服务端角色）
    zmq_bind(publisher, "tcp://*:5556");

    printf("Publisher bound to tcp://*:5556\n");
    sleep(1); // 等待订阅者连接（仅演示用）

    // 5. 发送消息
    for (int i = 0; i < 5; ++i) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Update %d", i);
        printf("Sending: %s\n", msg);
        zmq_send(publisher, msg, strlen(msg), 0); // 无 ZMQ_SNDMORE
        sleep(1);
    }

    // 6. 关闭
    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}
```

###### 📥 订阅者（Subscriber）—— 通常 `connect()`

```c
// sub.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);

    // 【可选】配置：设置接收高水位
    int hwm = 1000;
    zmq_setsockopt(subscriber, ZMQ_RCVHWM, &hwm, sizeof(hwm));

    // 【必须】订阅主题（空字符串 = 订阅所有）
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0); // 订阅所有消息

    // 连接到发布者
    zmq_connect(subscriber, "tcp://localhost:5556");

    printf("Subscriber connected. Waiting for messages...\n");

    // 接收消息
    for (int i = 0; i < 5; ++i) {
        char buffer[256];
        int size = zmq_recv(subscriber, buffer, sizeof(buffer) - 1, 0);
        if (size > 0) {
            buffer[size] = '\0';
            printf("Received: %s\n", buffer);
        }
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
```

> 🔔 注意：`ZMQ_SUB` 默认过滤所有消息，必须调用 `zmq_setsockopt(ZMQ_SUBSCRIBE, ...)` 才能收到数据。

---

##### ✅ 示例 2：REQ/REP 模式（同步请求-应答）

###### 🧑‍💻 客户端（Client）—— 使用 `REQ`，通常 `connect()`

```c
// client.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);

    // 【可选】设置 linger 时间（退出时等待未发送消息的时间，单位毫秒）
    int linger = 1000; // 1秒
    zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));

    zmq_connect(requester, "tcp://localhost:5555");

    for (int i = 0; i < 3; ++i) {
        const char *msg = "Hello";
        zmq_send(requester, msg, strlen(msg), 0);

        char buffer[256];
        int size = zmq_recv(requester, buffer, sizeof(buffer) - 1, 0);
        if (size > 0) {
            buffer[size] = '\0';
            printf("Received reply %d: [%s]\n", i, buffer);
        }
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);
    return 0;
}
```

###### 🤖 服务端（Server）—— 使用 `REP`，通常 `bind()`

```c
// server.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);

    // 【可选】设置接收缓冲区高水位
    int hwm = 100;
    zmq_setsockopt(responder, ZMQ_RCVHWM, &hwm, sizeof(hwm));

    zmq_bind(responder, "tcp://*:5555");
    printf("Server listening on port 5555...\n");

    while (1) {
        char buffer[256];
        int size = zmq_recv(responder, buffer, sizeof(buffer) - 1, 0);
        if (size > 0) {
            buffer[size] = '\0';
            printf("Received request: [%s]\n", buffer);

            // 模拟处理
            sleep(1);

            // 回复
            const char *reply = "World";
            zmq_send(responder, reply, strlen(reply), 0);
        }
    }

    // 实际中可能不会执行到这里
    zmq_close(responder);
    zmq_ctx_destroy(context);
    return 0;
}
```

> ⚠️ 注意：`REQ`/`REP` 必须严格遵循 **send → recv → send → recv...** 顺序，否则会报错（EFSM）。

---

##### ✅ 总结：关键点回顾

| 步骤 | 函数 | 说明 |
|------|------|------|
| 创建上下文 | `zmq_ctx_new()` | 全局资源管理器 |
| 创建套接字 | `zmq_socket(ctx, type)` | 如 `ZMQ_REQ`, `ZMQ_SUB` |
| 配置选项 | `zmq_setsockopt()` | HWM、SUBSCRIBE、LINGER 等 |
| 绑定/连接 | `zmq_bind()` / `zmq_connect()` | 顺序无关，自动重连 |
| 发送/接收 | `zmq_send()` / `zmq_recv()` | 面向完整消息 |
| 清理资源 | `zmq_close()` + `zmq_ctx_destroy()` | 防止内存泄漏 |


---

### 四、传输层与地址格式

- 支持多种传输机制，通过统一 URI 表示：
  - `inproc://name`：线程间（最快，无拷贝）
  - `ipc:///tmp/file`：进程间（Unix 域套接字）
  - `tcp://host:port`：跨网络通信
- 同一套接字可绑定多个传输端点（如同时监听 TCP 和 IPC）。

---

#### 示例

##### ✅ 示例 1：`inproc://` —— 线程间通信（最快，零拷贝）

```c
// inproc_example.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void* worker_thread(void *ctx) {
    // 在子线程中创建套接字并连接到 inproc 地址
    void *receiver = zmq_socket(ctx, ZMQ_PAIR);
    zmq_connect(receiver, "inproc://worker");

    char buffer[256];
    int size = zmq_recv(receiver, buffer, sizeof(buffer) - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        printf("Worker received: %s\n", buffer);
    }

    zmq_close(receiver);
    return NULL;
}

int main(void) {
    void *context = zmq_ctx_new();

    // 主线程创建 PAIR 套接字并绑定
    void *sender = zmq_socket(context, ZMQ_PAIR);
    zmq_bind(sender, "inproc://worker");

    // 启动工作线程
    pthread_t thread;
    pthread_create(&thread, NULL, worker_thread, context);

    // 发送消息
    const char *msg = "Hello from main thread!";
    zmq_send(sender, msg, strlen(msg), 0);

    pthread_join(thread, NULL);

    zmq_close(sender);
    zmq_ctx_destroy(context);
    return 0;
}
```

> 🔹 `inproc` 要求**共享同一个 ZeroMQ 上下文（context）**，仅限同一进程内的线程间通信。  
> 🔹 无需序列化/反序列化，性能极高。

---

##### ✅ 示例 2：`ipc:///` —— 进程间通信（Unix 域套接字）

> ⚠️ 注意：`ipc` 仅在类 Unix 系统（Linux/macOS）上有效，Windows 不支持。

###### 📥 服务端（ipc_server.c）

```c
// ipc_server.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REP);

    // 绑定到 IPC 路径（文件路径）
    zmq_bind(socket, "ipc:///tmp/zmq_ipc_test");

    printf("IPC server listening on ipc:///tmp/zmq_ipc_test\n");

    char buffer[256];
    int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        printf("Received: %s\n", buffer);
        zmq_send(socket, "ACK", 3, 0);
    }

    sleep(1); // 留时间给客户端接收

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
```

###### 📤 客户端（ipc_client.c）

```c
// ipc_client.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);

    zmq_connect(socket, "ipc:///tmp/zmq_ipc_test");

    zmq_send(socket, "PING", 4, 0);

    char buffer[256];
    int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        printf("Server replied: %s\n", buffer);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
```

> 🔹 IPC 使用本地文件系统路径（如 `/tmp/xxx`），比 TCP 快，但仅限本机进程间。

---

##### ✅ 示例 3：`tcp://` —— 跨网络通信（最通用）

###### 🌐 服务端（tcp_server.c）

```c
// tcp_server.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REP);
    
    // 监听所有接口的 5555 端口
    zmq_bind(socket, "tcp://*:5555");
    printf("TCP server listening on port 5555\n");

    char buffer[256];
    while (1) {
        int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (size > 0) {
            buffer[size] = '\0';
            printf("Received: %s\n", buffer);
            zmq_send(socket, "OK", 2, 0);
            break; // 只处理一次
        }
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
```

###### 🖥️ 客户端（tcp_client.c）

```c
// tcp_client.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REQ);

    // 连接到本地或远程主机
    zmq_connect(socket, "tcp://localhost:5555");

    zmq_send(socket, "Hello TCP!", strlen("Hello TCP!"), 0);

    char buffer[256];
    int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        printf("Reply: %s\n", buffer);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
```

> 🔹 `tcp://host:port` 支持跨机器通信，是最常用的传输方式。

---

##### ✅ 示例 4：**一个套接字绑定多个传输端点**

ZeroMQ 允许**同一个套接字同时绑定多个地址**，例如同时监听 TCP 和 IPC：

```c
// multi_transport.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_REP);

    // 同时绑定到 TCP 和 IPC
    zmq_bind(socket, "tcp://*:5555");
    zmq_bind(socket, "ipc:///tmp/multi_ipc");

    printf("Listening on:\n");
    printf("  - tcp://*:5555\n");
    printf("  - ipc:///tmp/multi_ipc\n");

    // 接收任意来源的消息
    char buffer[256];
    int size = zmq_recv(socket, buffer, sizeof(buffer) - 1, 0);
    if (size > 0) {
        buffer[size] = '\0';
        printf("Received: %s\n", buffer);
        zmq_send(socket, "Multi-OK", 9, 0);
    }

    sleep(2); // 留时间回复

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
```

> ✅ 客户端可以选择通过 `tcp://localhost:5555` 或 `ipc:///tmp/multi_ipc` 任一方式连接，服务端都能正确响应。

---

##### 📌 总结：传输层特性对比

| 传输类型 | URI 格式 | 范围 | 性能 | 跨平台 |
|--------|--------|------|------|--------|
| `inproc` | `inproc://name` | 同进程线程间 | ⚡ 极高（零拷贝） | ✅ 是 |
| `ipc`    | `ipc:///path`   | 同主机进程间 | 🚀 高（内核缓冲） | ❌ 仅 Unix |
| `tcp`    | `tcp://host:port` | 跨网络 | 🌐 中等（有协议开销） | ✅ 是 |



---

### 五、消息模型：原子性与多帧消息

- **每条消息是原子的**：要么完整送达，要么丢失。
- **多部分消息**（Multi-part Message）：
  - 允许将逻辑消息拆分为多个帧（如 `[Identity][Header][Body]`）
  - 使用 `ZMQ_SNDMORE` 标志发送中间帧
  - 接收时通过 `ZMQ_RCVMORE` 判断是否还有后续帧
- 多帧是实现**高级路由**（如 ROUTER 身份帧）的基础。

#### 示例
##### ✅ 示例目标
- 发送一个包含 **3 个帧** 的逻辑消息：`[Header][Command][Payload]`
- 使用 `ZMQ_SNDMORE` 标志发送前两帧
- 接收端循环读取所有帧，直到 `ZMQ_RCVMORE` 返回 false
- 验证：**要么收到全部 3 帧，要么一帧都收不到**（体现原子性）

> 💡 我们使用 `DEALER`/`ROUTER` 或 `PAIR`/`PAIR` 模式均可。这里选用 **`PAIR`**（简单、点对点、适合演示），也可替换为 `DEALER`/`ROUTER`。


```c
// multi_threaded.c
#include <unistd.h>
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void* receiver_func(void *ctx) {
    void *sock = zmq_socket(ctx, ZMQ_PAIR);
    zmq_connect(sock, "inproc://test");

    while (1) {
        char buf[256];
        int n = zmq_recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        printf("Frame: [%s]\n", buf);
        sleep(1);

        int64_t more;
        size_t sz = sizeof(more);
        zmq_getsockopt(sock, ZMQ_RCVMORE, &more, &sz);
        if (!more) {
            printf("→ Message complete.\n");
            break;
        }
    }
    zmq_close(sock);
    return NULL;
}

int main(void) {
    void *ctx = zmq_ctx_new();

    // 启动接收线程
    pthread_t th;
    pthread_create(&th, NULL, receiver_func, ctx);

    // 主线程作为发送方
    void *sender = zmq_socket(ctx, ZMQ_PAIR);
    zmq_bind(sender, "inproc://test");

    sleep(1); // 确保接收方 ready（仅演示用）

    // zmq_send(sender, "ID", 2, 0);
    // zmq_send(sender, "REQ", 3, 0); //  必须用 ZMQ_SNDMORE 否则对方接受一次后调用 ZMQ_RCVMORE 查询的结果为已完成
    zmq_send(sender, "ID", 2, ZMQ_SNDMORE);
    zmq_send(sender, "REQ", 3, ZMQ_SNDMORE);
    zmq_send(sender, "{\"action\":\"start\"}", 18, 0);

    pthread_join(th, NULL);

    zmq_close(sender);
    zmq_ctx_destroy(ctx);
    return 0;
}
```

---

#### 🔍 关键点说明

##### 1. **原子性保证**
- ZeroMQ 保证：**一个多帧消息要么全部送达，要么完全不送达**。
- 如果网络中断发生在第2帧之后，接收方**不会看到前两帧**（它们会被丢弃）。
- 这对协议设计至关重要（例如：身份+命令+数据 必须完整才有效）。

##### 2. **ZMQ_SNDMORE**
- 用于标记“当前帧不是最后一帧”。
- 只能用于 `zmq_send()` 的 `flags` 参数。
- 最后一帧必须传 `0`（或 `ZMQ_DONTWAIT` 等其他标志，但不能有 `SNDMORE`）。

##### 3. **ZMQ_RCVMORE**
- 调用 `zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &size)`。
- 若 `more == 1`，表示刚收到的帧后面还有帧。
- 必须在下一次 `zmq_recv()` 之前检查，否则状态会重置。

##### 4. **典型应用场景**
- **ROUTER 套接字**：自动在消息前插入 **身份帧（Identity Frame）**，形成 `[Identity][Empty][User Data]` 多帧结构。
- **自定义协议**：如 `[Version][Type][Length][Payload]`
- **元数据+内容分离**：如 `[UserID][Timestamp][Event]`

---

### 六、异步与非阻塞特性

- ZeroMQ 在后台 I/O 线程中处理网络通信，**主线程不会阻塞**。
- 应用可通过 `zmq_poll()` 实现**事件驱动编程**，同时监听多个套接字或文件描述符。

---

#### 示例
##### 示例1

我们构建一个客户端，它同时：
1. **监听来自服务端的广播消息**（通过 `SUB` 套接字）
2. **定期向服务端发送心跳请求**（通过 `REQ` 套接字）

使用 `zmq_poll()` 轮询两个套接字，避免阻塞，并在任意套接字就绪时立即处理。

```c
// async_poll_full.c
#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

// 模拟服务端线程
void* server_thread_func(void *ctx) {
    // PUB 套接字：广播数据
    void *pub = zmq_socket(ctx, ZMQ_PUB);
    zmq_bind(pub, "inproc://data_pub");

    // REP 套接字：响应控制请求
    void *rep = zmq_socket(ctx, ZMQ_REP);
    zmq_bind(rep, "inproc://ctrl_rep");

    zmq_pollitem_t items[] = {
        { rep, 0, ZMQ_POLLIN, 0 }
    };

    int count = 0;
    while (count < 5) {
        // 发送广播（每 800ms）
        char msg[64];
        snprintf(msg, sizeof(msg), "MarketData-%d", count);
        zmq_send(pub, msg, strlen(msg), 0);
        printf("[Server] Broadcast: %s\n", msg);

        // 轮询 REP 请求（非阻塞）
        if (zmq_poll(items, 1, 200) > 0 && (items[0].revents & ZMQ_POLLIN)) {
            char req[256];
            int n = zmq_recv(rep, req, sizeof(req) - 1, 0);
            if (n > 0) {
                req[n] = '\0';
                printf("[Server] Got request: %s\n", req);
                zmq_send(rep, "ACK", 3, 0);
                count++;
            }
        }
    }

    zmq_close(pub);
    zmq_close(rep);
    return NULL;
}

int main(void) {
    void *context = zmq_ctx_new();

    // 启动服务端线程
    pthread_t server_th;
    pthread_create(&server_th, NULL, server_thread_func, context);

    sleep(1); // 确保服务端 bind 完成

    // === 客户端逻辑 ===
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "inproc://data_pub");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "inproc://ctrl_rep");

    printf("🚀 Client started. Polling for events...\n");

    int sent = 0;
    while (sent < 5) {
        // 发送请求（REQ 必须 send → recv）
        char req[32];
        snprintf(req, sizeof(req), "Ping-%d", sent);
        zmq_send(requester, req, strlen(req), 0);
        sent++;

        // 轮询两个套接字：SUB 和 REQ
        zmq_pollitem_t items[] = {
            { subscriber, 0, ZMQ_POLLIN, 0 },
            { requester,  0, ZMQ_POLLIN, 0 }
        };

        // 等待最多 2000ms
        zmq_poll(items, 2, 2000);

        // 处理广播消息（可能有多条）
        while (1) {
            char buf[256];
            int n = zmq_recv(subscriber, buf, sizeof(buf) - 1, ZMQ_DONTWAIT);
            if (n <= 0) break;
            buf[n] = '\0';
            printf("📢 Async data: %s\n", buf);
        }

        // 处理应答
        if (items[1].revents & ZMQ_POLLIN) {
            char buf[256];
            int n = zmq_recv(requester, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                printf("✅ Control reply: %s\n", buf);
            }
        }
    }

    // 清理
    zmq_close(subscriber);
    zmq_close(requester);
    pthread_join(server_th, NULL);
    zmq_ctx_destroy(context);
    return 0;
}
```

---

#### 🔍 关键机制说明

### 1. **`zmq_poll()` 的作用**
- 类似 Unix 的 `poll()` 或 `select()`，但专为 ZeroMQ 套接字优化。
- 可同时监听多个套接字的 **可读（`ZMQ_POLLIN`）** 或 **可写（`ZMQ_POLLOUT`）** 事件。
- 支持超时（毫秒），实现非阻塞轮询。

### 2. **非阻塞接收技巧**
- 使用 `ZMQ_DONTWAIT` 标志配合 `zmq_recv()` 可实现完全非阻塞读取。
- 或通过 `zmq_poll()` 先检测是否可读，再安全调用 `recv`。

### 3. **I/O 线程模型**
- 所有网络收发由 ZeroMQ 内部 I/O 线程处理。
- 应用主线程只负责业务逻辑和消息处理，**永不阻塞在网络 I/O 上**。


---

### 七、常见陷阱与最佳实践

- **不要滥用 `PAIR`**：仅适用于线程间，不适用于进程或网络。
- **`SUB` 默认丢弃所有消息**：必须调用 `zmq_setsockopt(ZMQ_SUBSCRIBE, ...)` 订阅。
- **`REQ`/`REP` 顺序不可乱**：必须“send → recv → send → recv...”，否则报错（EFSM）。
- **资源必须显式释放**：忘记 `zmq_close()` 或 `zmq_ctx_destroy()` 会导致内存泄漏。

#### `REQ`/`REP` 顺序不可乱

在 ZeroMQ 中，**`REQ`（请求）和 `REP`（应答）套接字是严格状态机驱动的**，它们内部维护了一个**有限状态机（Finite State Machine, FSM）**，强制通信双方遵循固定的交互顺序：

> **`REQ` 必须：send → recv → send → recv → …**  
> **`REP` 必须：recv → send → recv → send → …**

如果违反这个顺序（例如 `REQ` 连续发送两次，或 `REP` 先发送），ZeroMQ 会返回错误码 **`EFSM`（Operation cannot be accomplished in current state）**，表示“当前状态下无法执行该操作”。

---

##### 🔍 为什么这样设计？

- **简化协议逻辑**：确保每次请求都有且仅有一个响应，避免消息错乱。
- **自动处理底层细节**：如重连、消息边界、身份路由等，但代价是限制了使用方式。
- **防止常见错误**：比如客户端重复发请求而未等回复，导致服务端状态混乱。

---

##### ✅ 正确示例：标准 REQ/REP 循环

```c
// client.c (正确)
void *req = zmq_socket(context, ZMQ_REQ);
zmq_connect(req, "tcp://localhost:5555");

zmq_send(req, "Hello", 5, 0);        // ✅ 第1步：send
char buf[256];
zmq_recv(req, buf, 255, 0);          // ✅ 第2步：recv
// 可继续下一轮：send → recv ...
```

```c
// server.c (正确)
void *rep = zmq_socket(context, ZMQ_REP);
zmq_bind(rep, "tcp://*:5555");

char buf[256];
zmq_recv(rep, buf, 255, 0);          // ✅ 第1步：recv
zmq_send(rep, "World", 5, 0);        // ✅ 第2步：send
// 下一轮：recv → send ...
```

---

##### ❌ 错误示例 1：REQ 连续发送两次

```c
// client_bad1.c
zmq_send(req, "Msg1", 4, 0);   // OK
zmq_send(req, "Msg2", 4, 0);   // ❌ 错误！
```

**结果**：
- 第二次 `zmq_send()` 返回 `-1`
- `errno` 被设为 **`EFSM`**
- 程序需检查返回值并处理错误

---

##### ❌ 错误示例 2：REP 先发送

```c
// server_bad.c
zmq_send(rep, "Hi", 2, 0);     // ❌ 错误！REP 初始状态只能 recv
```

**结果**：同样触发 `EFSM` 错误。

---

##### 🛠️ 完整可运行的错误演示代码

###### client_double_send.c（故意出错）

```c
#include <zmq.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    void *ctx = zmq_ctx_new();
    void *req = zmq_socket(ctx, ZMQ_REQ);
    zmq_connect(req, "tcp://localhost:5555");

    printf("Sending first message...\n");
    int rc = zmq_send(req, "Hello", 5, 0);
    if (rc == -1) {
        fprintf(stderr, "First send failed: %s\n", strerror(errno));
        return 1;
    }

    printf("Sending second message (should fail)...\n");
    rc = zmq_send(req, "Again", 5, 0);  // ❌ 违反状态机！
    if (rc == -1) {
        fprintf(stderr, "Second send failed with errno=%d: %s\n", zmq_errno(), zmq_strerror(zmq_errno()));
        // 在 Linux/macOS 上，errno == EFSM (通常为 156 或自定义值)
        // 注意：ZeroMQ 的 EFSM 可能不是标准 errno，需用 zmq_errno()
    }

    zmq_close(req);
    zmq_ctx_destroy(ctx);
    return 0;
}
```

##### 🔄 如何安全地实现“多次请求”？

如果你需要连续发送多个请求，**必须在每次 send 后 recv 一次**：

```c
for (int i = 0; i < 3; i++) {
    char req[32];
    snprintf(req, sizeof(req), "Request-%d", i);
    zmq_send(req_sock, req, strlen(req), 0);

    char rep[256];
    zmq_recv(req_sock, rep, sizeof(rep) - 1, 0); // 必须 recv！
    printf("Reply: %s\n", rep);
}
```

或者改用更灵活的套接字类型（如 `DEALER`/`ROUTER`），但需自行管理消息边界和路由。

---

##### ✅ 总结

| 套接字 | 允许的操作序列 | 违反后果 |
|--------|----------------|--------|
| `REQ`  | send → recv → send → recv … | 第二次 send 报 `EFSM` |
| `REP`  | recv → send → recv → send … | 第一次 send 报 `EFSM` |

> ⚠️ **记住口诀**：  
> **REQ 先“说”，REP 先“听”；一来一回，不能乱动。**

这种限制虽然看似严格，但正是它保证了请求-应答模式的**简单性与可靠性**。若需更复杂的交互（如异步、多路复用），应选择 `DEALER`、`ROUTER` 或结合 `zmq_poll()` 构建状态机。


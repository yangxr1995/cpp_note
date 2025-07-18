#include <csignal>
#include <cstddef>
#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <netinet/in.h>
#include <sys/socket.h>
using namespace std;

void listen_cb(struct evconnlistener *listener, evutil_socket_t ev_socket, struct sockaddr *addr, int socklen, void *args)
{
    cout << "listen_cb" << endl;
}

int main (int argc, char *argv[]) {

    cout << "test libevent" << endl;
    event_base * base = event_base_new();
    if (base) {
        cout << "event_base_new success" << endl;
    }

    // SIGPIPE:写已关闭的socket/管道
    // 忽略SIGPIPE，避免程序莫名死掉
    signal(SIGPIPE, SIG_IGN);

    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);
    // 地址重用，listener关闭同时关闭socket
    int flags = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE;

    evconnlistener *listener = evconnlistener_new_bind(
            base,    // libevent上下文
            listen_cb,  // 连接后的回调函数
            NULL,       // 回调函数的参数
            flags,      // opt
            10,         // 连接队列大小
            (struct sockaddr *)&addr, // 监听地址
            sizeof(addr));

    // evconnlistener_new(base, listen_cb, base, flags , 10 , fd);
    
    event_base_dispatch(base); // 事件分发

    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}

#include "event2/buffer.h"
#include "event2/event_compat.h"
#include "event2/event_struct.h"
#include "event2/util.h"
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <event2/event.h>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
using namespace std;

// 处理超时，断开连接，接受数据
void read_cb(evutil_socket_t fd, short flags, void *arg) {

    event *ev = (event *)arg;
    char buf[5];
    int cnt;

    if (flags & EV_TIMEOUT) {
        cout << "timeout" << endl;
        goto __end__;
    }

    if ((cnt = recv(fd, buf, sizeof(buf), 0)) < 0) {
        cerr << "recv err: " << strerror(errno) << endl;
        goto __end__;
    }
    if (cnt == 0) {
        cout << "cnt == 0" << endl;
        goto __end__;
    }

    cout << buf << flush;
    return ;

__end__:
    event_free(ev);
    evutil_closesocket(fd);
    return ;
}

void listen_cb(evutil_socket_t fd, short flags, void *arg) {
    event_base *base = (event_base *)arg;

    int cfd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if ((cfd = accept(fd, (struct sockaddr *)&addr, &addr_len)) < 0) {
        cerr << "accept err : " << strerror(errno) << endl;
        return;
    }
    char ip[16];
    evutil_inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    cout << "client connect from " << ip << ":" << ntohs(addr.sin_port) << endl;

    evutil_make_socket_nonblocking(cfd);

    event *ev = event_new(base, cfd, EV_READ | EV_PERSIST, read_cb, event_self_cbarg());
    struct timeval tv = {10, 0};
    event_add(ev, &tv);

}

int main (int argc, char *argv[]) {

    event_base *base = event_base_new();

    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);
    if (::bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "bind err: " << strerror(errno) << endl;
        return -1;
    }

    listen(fd, 3);

    evutil_make_socket_nonblocking(fd);
    evutil_make_listen_socket_reuseable(fd);

    event *ev = event_new(base, fd, EV_READ | EV_PERSIST, listen_cb, base);
    event_add(ev, NULL);

    event_base_loop(base, 0);

    event_base_dispatch(base);


    return 0;
}

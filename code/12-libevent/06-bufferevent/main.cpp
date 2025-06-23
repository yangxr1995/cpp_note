#include <bits/types/struct_timeval.h>
#include <cstring>
#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

static unsigned int timeout_cnt = 0;

// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;
    char buf[256];
    int cnt;
    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    buf[cnt] = 0;
    if (strstr(buf, "quit")) {
        cout << "quit" << endl;
        bufferevent_free(bev);
        return;
    }
    cout << "cnt : " << cnt << endl;
    cout << "--" << buf << "--" << flush;
    timeout_cnt = 0;
    // bufferevent_write(bev, "OK", 3);
}

// 三次握手完成后, 调用bev_write_cb
void bev_write_cb(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;

}

// 四次挥手后，调用bev_event_cb
void bev_event_cb(struct bufferevent *bev, short what, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;
    if (what & BEV_EVENT_TIMEOUT) {
        cout << "BEV_EVENT_TIMEOUT :";
        if (what & BEV_EVENT_READING) {
            cout << "BEV_EVENT_READING" << endl;

            if (timeout_cnt >= 3) {
                cout << "close connect" << endl;
                bufferevent_free(bev);
            }
            else {
                timeout_cnt++;
                bufferevent_enable(bev, EV_READ);
            }
        }
        if (what & BEV_EVENT_WRITING) {
            cout << "BEV_EVENT_WRITING" << endl;
        }
    }

    if (what & BEV_EVENT_EOF) {
        cout << "BEV_EVENT_EOF" << endl;
        bufferevent_free(bev);
    }

    if (what & BEV_EVENT_ERROR) {
        cout << "BEV_EVENT_ERROR" << endl;
        bufferevent_free(bev);
    }
}

void listen_cb(struct evconnlistener *listener, evutil_socket_t cfd, 
        struct sockaddr *addr, int socklen, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;
    event_base *base = (event_base *)arg;

    // 1. 对通信套接字绑定bufferevent
    struct bufferevent *bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);

    // 2. 设置参数

    // 设置读缓存
    // 数据不足高水位时，从fd读取
    // 数据超过低水位时，调用读回调
    bufferevent_setwatermark(bev, EV_READ, 15, 20);

    // 设置写缓存
    // 数据不足低水位时，调用写回调
    // 高水位无效
    bufferevent_setwatermark(bev, EV_WRITE, 20, 0);

    // 超过5s没有新的数据到输入缓存，则调用读回调
    struct timeval tv_read = {5, 0};
    bufferevent_set_timeouts(bev, &tv_read, NULL);

    // 3. 设置回调函数
    bufferevent_setcb(bev, bev_read_cb, bev_write_cb, bev_event_cb, bev);

    // 4. 开启读写监听
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int main (int argc, char *argv[]) {
    
    event_base *base = event_base_new();

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);

    struct evconnlistener *listener = evconnlistener_new_bind(base, listen_cb, base, 
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 3, (struct sockaddr *)&addr , sizeof(addr));

    event_base_dispatch(base);

    return 0;
}

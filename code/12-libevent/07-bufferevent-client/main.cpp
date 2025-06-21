#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace std;

static string file_data;
static int recv_cnt;
static int send_cnt;

void bev_read_cb_client(struct bufferevent *bev, void *ctx) {
    cout << "client : " << __PRETTY_FUNCTION__ << endl;
}

void bev_write_cb_client(struct bufferevent *bev, void *ctx) {
    cout << "client : " << __PRETTY_FUNCTION__ << endl;

    FILE *fp = fopen("./main.cpp", "r");
    if (fp == NULL) {
        cerr << "fopen err : " << strerror(errno) << endl;
        return;
    }
    char buf[1024];
    int cnt, cnt2;
    while ((cnt = fread(buf, 1, sizeof(buf), fp)) > 0) {
        // bufferevent 对fd进行封装，读写直接通过bufferevent
        cout << "client cnt : " << cnt << endl;
        if ((cnt2 = bufferevent_write(bev, buf, cnt)) < 0) {
            cerr << "bufferevent_write err : " << endl;
            break;
        }
        send_cnt += cnt;
    }
    fclose(fp);

    bufferevent_free(bev);
}

// bufferevent将数据事件和其他事件分离
void bev_event_cb_client(struct bufferevent *bev, short what, void *ctx) {
    cout << "client : " ;
    if (what & BEV_EVENT_CONNECTED) {
        cout << "BEV_EVENT_CONNECTED" << endl;
        bufferevent_trigger(bev, EV_WRITE, 0);
    }
    if (what & BEV_EVENT_EOF) {
        cout << "BEV_EVENT_EOF" << endl;
    }
}


// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb(struct bufferevent *bev, void *ctx) {
    cout << "server : " << __PRETTY_FUNCTION__ << endl;
    char buf[4096];
    int cnt;
    // 读缓存的高水位会影响一次最多读多少
    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    buf[cnt] = 0;
    file_data += buf;
    recv_cnt += cnt;
    cout << "cnt : " << cnt << endl;
    // cout << buf << flush;
    // bufferevent_write(bev, "OK", 3);
}

// 三次握手完成后, 调用bev_write_cb
void bev_write_cb(struct bufferevent *bev, void *ctx) {
    cout << "server : " << __PRETTY_FUNCTION__ << endl;

}

// 四次挥手后，调用bev_event_cb
void bev_event_cb(struct bufferevent *bev, short what, void *ctx) {
    cout << "server : " << __PRETTY_FUNCTION__ << endl;
    if (what & BEV_EVENT_TIMEOUT) {
        cout << "server : BEV_EVENT_TIMEOUT | ";
        if (what & BEV_EVENT_READING) {
            cout << "BEV_EVENT_READING" << endl;
        }
        if (what & BEV_EVENT_WRITING) {
            cout << "BEV_EVENT_WRITING" << endl;
        }
        bufferevent_free(bev);
    }

    if (what & BEV_EVENT_EOF) {
        cout << "server : BEV_EVENT_EOF | ";
        if (what & BEV_EVENT_WRITING) {
            cout << "BEV_EVENT_WRITING" << endl;
        }
        if (what & BEV_EVENT_READING) {
            cout << "BEV_EVENT_READING" << endl;
        }

        // cout << file_data << endl;
        // cout << "send_cnt : " << send_cnt << endl;
        // cout << "recv_cnt : " << recv_cnt << endl;

        bufferevent_free(bev);
    }

}

void listen_cb(struct evconnlistener *listener, evutil_socket_t cfd, 
        struct sockaddr *addr, int socklen, void *arg) {
    cout << "server : " << __PRETTY_FUNCTION__ << endl;
    event_base *base = (event_base *)arg;

    struct bufferevent *bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);

    // 设置读缓存
    // 数据不足高水位时，从fd读取
    // 数据超过低水位时，调用读回调
    bufferevent_setwatermark(bev, EV_READ, 10, 20);

    // 设置写缓存
    // 数据不足低水位时，调用写回调
    // 高水位无效
    bufferevent_setwatermark(bev, EV_WRITE, 5, 0);

    // 超过5s没有新的数据到输入缓存，则调用读回调
    // struct timeval tv_read = {5, 0};
    // bufferevent_set_timeouts(bev, &tv_read, NULL);

    bufferevent_setcb(bev, bev_read_cb, bev_write_cb, bev_event_cb, bev);

    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

int main (int argc, char *argv[]) {
    
    event_base *base = event_base_new();

    // server
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(5001);

        struct evconnlistener *listener = evconnlistener_new_bind(base, listen_cb, base, 
                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 3, (struct sockaddr *)&addr , sizeof(addr));
    }

    // client
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(5001);

        struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, bev_read_cb_client , bev_write_cb_client , bev_event_cb_client , base);
        bufferevent_enable(bev, EV_READ | EV_WRITE);

        if (bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            cerr << "bufferevent_socket_connect failed : " << strerror(errno) << endl;
            return -1;
        }
    }

    event_base_dispatch(base);

    return 0;
}

#include <bits/types/struct_timeval.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>

using namespace std;


enum bufferevent_filter_result bev_filter_input(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    char buf[256];
    int cnt;
    cnt = evbuffer_remove(src, buf, sizeof(buf));
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

enum bufferevent_filter_result bev_filter_output(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {
    cout <<__PRETTY_FUNCTION__ << endl;

    char buf[256];
    int cnt;
    cnt = evbuffer_remove(src, buf, sizeof(buf));
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb_client(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;
    char buf[256];
    int cnt;
    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    bufferevent_write(bev, buf, cnt);
}

const char *g_send_file_name;

void bev_write_cb_client(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;


    // vector<char> read_file_with_header("./1.tmp");
}

// 四次挥手后，调用bev_event_cb
void bev_event_cb_client(struct bufferevent *bev, short what, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;
    if (what & BEV_EVENT_TIMEOUT) {
        cout << "BEV_EVENT_TIMEOUT :";
        if (what & BEV_EVENT_READING) {
            cout << "BEV_EVENT_READING" << endl;
        }
        if (what & BEV_EVENT_WRITING) {
            cout << "BEV_EVENT_WRITING" << endl;
        }
    }
    else if (what & BEV_EVENT_ERROR) {
        cout << "BEV_EVENT_ERROR" << endl;
    }
    else if (what & BEV_EVENT_EOF) {       // socket: RST, FIN
        cout << "BEV_EVENT_EOF" << endl; 
    }
    else if (what & BEV_EVENT_CONNECTED) {
        cout << "BEV_EVENT_CONNECTED" << endl;

        struct bufferevent *bev_filter = bufferevent_filter_new(bev, bev_filter_input, bev_filter_output, BEV_OPT_CLOSE_ON_FREE, NULL, NULL);
        bufferevent_setcb(bev_filter, bev_read_cb_client, bev_write_cb_client, bev_event_cb_client, bev_filter);
        bufferevent_enable(bev_filter, EV_WRITE | EV_READ);
        bufferevent_trigger(bev_filter, EV_WRITE, 0);
    }
}



// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << endl;
    char buf[256];
    int cnt;
    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    buf[cnt] = 0;
    // if (strstr(buf, "quit")) {
    //     cout << "quit" << endl;
    //     bufferevent_free(bev);
    //     return;
    // }
    cout << "cnt : " << cnt << endl;
    cout << buf << flush;
    // bufferevent_write(bev, buf, cnt);
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
        }
        if (what & BEV_EVENT_WRITING) {
            cout << "BEV_EVENT_WRITING" << endl;
        }
    }

}

enum bufferevent_filter_result bev_filter_input_client (
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    cout <<__PRETTY_FUNCTION__ << endl;

    char buf[256];
    int cnt;
    cnt = evbuffer_remove(src, buf, sizeof(buf));
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

enum bufferevent_filter_result bev_filter_output_client (
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {
    cout <<__PRETTY_FUNCTION__ << endl;

    char buf[256];
    int cnt;
    cnt = evbuffer_remove(src, buf, sizeof(buf));
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}





void listen_cb(struct evconnlistener *listener, evutil_socket_t cfd, 
        struct sockaddr *addr, int socklen, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;
    event_base *base = (event_base *)arg;

    struct bufferevent *bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);

    // 给bev设置filter，在读回调前调用 filter 的 input，写回调后调用 filter的 output
    struct bufferevent *bev_filter = bufferevent_filter_new(bev, 
            bev_filter_input, 
            bev_filter_output, 
            BEV_OPT_CLOSE_ON_FREE,  // 关闭filter，同时关闭bufferevent 
            NULL,  // 清理回调函数
            NULL   // 传给所有回调的参数
            );

    // NOTE: 所有回调，都是对顶层的bufferevent 进行设置
    bufferevent_setcb(bev_filter, bev_read_cb, bev_write_cb, bev_event_cb, bev);

    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}

int main (int argc, char *argv[]) {
    
    event_base *base = event_base_new();

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);

    if (argc == 2 && strncmp(argv[1], "-s", 2) == 0) {
        // server
        struct evconnlistener *listener = evconnlistener_new_bind(base, listen_cb, base, 
                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 3, (struct sockaddr *)&addr , sizeof(addr));
    }
    else {
        if (argc != 2) {
            cerr << "usage : " << argv[0] << " <file>" << endl;
            return -1;
        }
        g_send_file_name = argv[1];

        // client
        struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, bev_read_cb_client , bev_write_cb_client , bev_event_cb_client , bev);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
        bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr));
    }

    event_base_dispatch(base);

    return 0;
}


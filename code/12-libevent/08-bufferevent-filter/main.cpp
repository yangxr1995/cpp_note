#include <bits/types/struct_timeval.h>
#include <sys/stat.h>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace std;

const char *actor;

#define CLIENT_BUF_SZ 200
#define SERVER_BUF_SZ 20

class conn {
    public:
        conn(struct bufferevent *bev_filter = nullptr)
        :_bev_filter(bev_filter), _file_name("") {
            _is_ack_ok = false;
            _fp = nullptr;
        }
        void set_bev_filter(struct bufferevent *filter) {
            _bev_filter = filter;
        }
        void close_file() {
            fclose(_fp);
            _fp = nullptr;
        }
        void set_file_name(const string &name) {
            _file_name = name;
        }
        void set_file_size(size_t sz) {
            _file_sz = sz;
        }
        void set_remain_bytes(size_t sz) {
            _remain_bytes = sz;
        }
        size_t remain_bytes() {
            return _remain_bytes;
        }
        size_t remain_bytes_del(size_t bytes) {
            _remain_bytes -= bytes;
            return _remain_bytes;
        }
        void set_file(const string &name) {
            struct stat st;
            if (stat(name.c_str(), &st) < 0) {
                cout << "err : get file stat err " << name << " " << strerror(errno) << endl;
                exit(1);
            }
            _file_sz = st.st_size;
            _file_name = name;
        }
        bool is_ack_ok() {
            return _is_ack_ok;
        }
        void set_ack_ok(bool ack) {
            _is_ack_ok = ack;
        }
        size_t file_size() {
            return _file_sz;
        }
        string &file_name() {
            return _file_name;
        }
        size_t read_file(char *buf, size_t sz) {
            if (_fp == nullptr) {
                _fp = fopen(_file_name.c_str(), "r");
                if (_fp == nullptr) {
                    cout << "failed to open " << _file_name << " : " << strerror(errno) << endl;
                    return -1;
                }
            }
            return fread(buf, 1, sz, _fp);
        }
        size_t write_file(char *buf, size_t sz) {
            if (_fp == nullptr) {
                _fp = fopen(_file_name.c_str(), "w");
                if (_fp == nullptr) {
                    cout << "failed to open " << _file_name << " : " << strerror(errno) << endl;
                    return -1;
                }
            }
            return fwrite(buf, 1, sz, _fp);
        }
        ~conn() {
            if (_fp)
                fclose(_fp);
            bufferevent_free(_bev_filter);
        }
    private:
        struct bufferevent *_bev_filter;
        string _file_name;
        size_t _file_sz;
        size_t _remain_bytes;
        bool _is_ack_ok;
        FILE *_fp;
};

enum bufferevent_filter_result bev_filter_input_client(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    char buf[256];
    int cnt;
    conn *c = (conn *)ctx;

    cnt = evbuffer_remove(src, buf, sizeof(buf) -1);
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

enum bufferevent_filter_result bev_filter_output_client(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    char buf[256];
    int cnt;
    conn *c = (conn *)ctx;

    cnt = evbuffer_remove(src, buf, sizeof(buf) - 1);

    if (c->is_ack_ok()) {
        // 压缩
    }

    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

enum bufferevent_filter_result bev_filter_input_server(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    conn *c = (conn *)ctx;
    char buf[256];
    int cnt;

    cnt = evbuffer_remove(src, buf, sizeof(buf));

    if (!c->file_name().empty()) {
        // 解压
    }

    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}

enum bufferevent_filter_result bev_filter_output_server(
    struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    enum bufferevent_flush_mode mode, void *ctx) {

    conn *c = (conn *)ctx;
    char buf[256];
    int cnt;

    cnt = evbuffer_remove(src, buf, sizeof(buf) - 1);
    evbuffer_add(dst, buf, cnt);

    return BEV_OK;
}


// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb_client(struct bufferevent *bev, void *ctx) {
    char buf[CLIENT_BUF_SZ];
    int cnt;
    conn *c = (conn *)ctx;

    cout << __PRETTY_FUNCTION__ << endl;

    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    buf[cnt] = 0;

    if (strcmp(buf, "OK") == 0) {
        if (!c->is_ack_ok())
            c->set_ack_ok(true);
    }
    else {
        cout << "err: ack err";
        delete c;
    }

    cout << "server resp : " << buf << endl;

    cnt = c->read_file(buf, sizeof(buf));
    if (cnt < 0) {
        cout << "client : read file err" << endl;
        delete c;
        exit(1);
    }

    if (cnt == 0) {

        struct bufferevent *bev_underly = bufferevent_get_underlying(bev);
        struct evbuffer *output_evbuffer = bufferevent_get_output(bev_underly);
        if (evbuffer_get_length(output_evbuffer) > 0) {
            cout << "flush remain bytes";
            bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
            return ;
        }


        cout << "client : send finish" << endl;
        delete c;
        exit(0);
    }

    cout << "send file.. "  << cnt << " bytes"<< endl;

    bufferevent_write(bev, buf, cnt);
}

const char *g_send_file_name;

void bev_write_cb_client(struct bufferevent *bev, void *ctx) {
    cout << __PRETTY_FUNCTION__ << bev << endl;
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

        conn *c = new conn();

        c->set_file(g_send_file_name);

        struct bufferevent *bev_filter = bufferevent_filter_new(bev, bev_filter_input_client, bev_filter_output_client, BEV_OPT_CLOSE_ON_FREE, NULL, c);
        bufferevent_setcb(bev_filter, bev_read_cb_client, bev_write_cb_client, bev_event_cb_client, c);
        bufferevent_enable(bev_filter, EV_WRITE | EV_READ);

        c->set_bev_filter(bev_filter);

        char tmp[128] = {0};
        snprintf(tmp, sizeof(tmp), "%s:%ld", c->file_name().c_str(), c->file_size());
        cout << "send : " << tmp << endl;

        bufferevent_write(bev_filter, tmp, strlen(tmp));
    }
}

// 接收到对端用户数据后，调用bev_read_cb
void bev_read_cb_server(struct bufferevent *bev, void *ctx) {
    char buf[SERVER_BUF_SZ];
    int cnt;
    conn *c = (conn *)ctx;


    cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
    buf[cnt] = 0;

    if (c->file_name().empty()) {
        char name[64] , *p;
        size_t sz;

        if ((p = strchr(buf, ':')) == nullptr) {
            cout << "err : parse err " << buf << endl;
            delete c;
        }
        *p = 0;
        strcpy(name, buf);
        sz = atoi(p + 1);

        strcat(name, "2");

        cout << "recv : " << buf << endl;
        cout << "name : " << name << " , size : " << sz << endl;

        c->set_file_name(name);
        c->set_file_size(sz);
        c->set_remain_bytes(sz);

        bufferevent_write(bev, "OK", 2);

        return ;
    }

    struct evbuffer *input_evbuffer = bufferevent_get_input(bev);

    while (1) {
        cnt = c->write_file(buf, cnt);
        if (cnt < 0) {
            cout << "write file err" << endl;
            delete c;
            exit(1);
        }

        if (c->remain_bytes_del(cnt) == 0) {
            cout << "break : remain_bytes_del == 0" << endl;
            break;
        }

        if (evbuffer_get_length(input_evbuffer) == 0) {
            cout << "break : evbuffer_get_length == 0" << endl;
            break;
        }

        cout << "evbuffer_get_length : " << evbuffer_get_length(input_evbuffer) << endl;
        cout << "read buffer again" << endl;

        cnt = bufferevent_read(bev, buf , sizeof(buf) - 1);
        buf[cnt] = 0;
    } 

    bufferevent_write(bev, "OK", 2);

    if (c->remain_bytes() == 0) {
        delete c;
    }
}

// 三次握手完成后, 调用bev_write_cb
void bev_write_cb_server(struct bufferevent *bev, void *ctx) {
    // cout << __PRETTY_FUNCTION__ << endl;
}

// 四次挥手后，调用bev_event_cb
void bev_event_cb_server(struct bufferevent *bev, short what, void *ctx) {
    conn *c = (conn *)ctx;

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

    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        delete c;
    }

}

// 服务端处理：3次握手完成，处理新的连接
void listen_cb(struct evconnlistener *listener, evutil_socket_t cfd, 
        struct sockaddr *addr, int socklen, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;
    event_base *base = (event_base *)arg;

    conn *c = new conn();

    struct bufferevent *bev = bufferevent_socket_new(base, cfd, BEV_OPT_CLOSE_ON_FREE);
    // 给bev设置filter，在读回调前调用 filter 的 input，写回调后调用 filter的 output
    struct bufferevent *bev_filter = bufferevent_filter_new(bev, 
            bev_filter_input_server, 
            bev_filter_output_server, 
            BEV_OPT_CLOSE_ON_FREE,  // 关闭filter，同时关闭bufferevent 
            NULL,  // 清理回调函数
            c   // 传给所有回调的参数
            );
    bufferevent_setcb(bev_filter, bev_read_cb_server, NULL, bev_event_cb_server, c);
    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);

    c->set_bev_filter(bev_filter);
}

int main (int argc, char *argv[]) {
    
    event_base *base = event_base_new();

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5001);

    if (argc == 2 && strncmp(argv[1], "-s", 2) == 0) {
        // server
        actor = "server";
        struct evconnlistener *listener = evconnlistener_new_bind(base, listen_cb, base, 
                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 3, (struct sockaddr *)&addr , sizeof(addr));
    }
    else {
        if (argc != 2) {
            cerr << "usage : " << argv[0] << " <file>" << endl;
            return -1;
        }
        g_send_file_name = argv[1];

        actor = "client";

        // client
        struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, bev_read_cb_client , NULL, bev_event_cb_client , bev);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
        bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr));
    }

    event_base_dispatch(base);

    return 0;
}


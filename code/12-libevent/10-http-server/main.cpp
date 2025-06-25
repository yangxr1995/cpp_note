#include "event2/buffer.h"
#include "event2/event_compat.h"
#include "event2/event_struct.h"
#include "event2/keyvalq_struct.h"
#include "event2/util.h"
#include <array>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <event2/http.h>
#include <event2/event.h>
#include <iterator>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
using namespace std;

#define WEBROOT "."
#define DEFAULT_FILE "/index.html"

void http_req_hander_submit_form(struct evhttp_request *req, void *arg) {
    cout << "submit-form hander" << endl;

    evhttp_send_reply(req, HTTP_OK, "", NULL);
}

void http_req_gen_handler(struct evhttp_request *req, void *arg) {
    cout << "http" << endl;
    // 1. 获得请求行
    // string uri( evhttp_request_get_uri(req));
    // cout << "uri : " << uri << endl;

    struct evhttp_uri *uri = evhttp_uri_parse(evhttp_request_get_uri(req));

    // 1.1 获得请求文件
    string filename = evhttp_uri_get_path(uri);
    if (filename.back() == '/' || filename.empty())
        filename = DEFAULT_FILE;
    filename = filename.substr(filename.rfind('/') + 1);
    cout << "file : " << filename << endl;

    // 1.2 获得方法
    string method = "unknow";
    switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET:
            method = "GET";
            break;
        case EVHTTP_REQ_POST:
            method = "POST";
            break;
        default:
            break;
    }
    cout << "method : " << method << endl;

    // 2. 获得请求头
    struct evkeyvalq *keyvalq = evhttp_request_get_input_headers(req);
    for (struct evkeyval *pos = keyvalq->tqh_first; pos != NULL; pos = pos->next.tqe_next) {
        cout << pos->key << ":" << pos->value << endl;
    }

    // 3. 获得请求体
    struct evbuffer *inbuf = evhttp_request_get_input_buffer(req);

    cout << "---------body----------" << endl;
    while (evbuffer_get_length(inbuf) > 0) {
        char buf[1024];
        int cnt = evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
        if (cnt < 0)
            break;
        buf[cnt] = 0;
        cout << buf << endl;
    }

    // 4. 处理响应
    cout << "--------resp----------" << endl;

    string filetype = filename.substr(filename.rfind('.') + 1);
    string content_type;

    if (filetype == "jpg" || filetype == "png") {
        content_type += "image/" + filetype;
    }
    else if (filetype == "html" || filetype == "css") {
        content_type += "text/" + filetype;
    }
    else  {
        content_type += "application/" + filetype;
    }
    struct evkeyvalq *outhead = evhttp_request_get_output_headers(req);
    evhttp_add_header(outhead, "Content-type", content_type.c_str());

    cout << "Content-type: " << content_type<< endl;

    // 4.1 构造响应体
    fstream file(filename, ios::in | ios::binary);

    if (!file.is_open()) {
        cout << "resp : 404" << endl;
        cout << "file : " << filename << endl;
        evhttp_send_reply(req, HTTP_NOTFOUND, "", NULL);
        return;
    }

    struct evbuffer *outbuf = evhttp_request_get_output_buffer(req);

    array<char, 1024> buf;
    while (file) {
        file.read(buf.data(), buf.size());
        if (file.gcount() > 0)
            evbuffer_add(outbuf, buf.data(), file.gcount());
    }

    if (!file.eof()) {
        cerr << "err : file read" << endl;
    }

    // 4.2 发送响应
    evhttp_send_reply(req, HTTP_OK, "", outbuf);
}

int main (int argc, char *argv[]) {

    event_base *base = event_base_new();

    // 0. 创建http上下文
    evhttp *http = evhttp_new(base);
    
    // 1. 确认绑定的地址
    if (evhttp_bind_socket(http, "0.0.0.0", 8080) < 0) {
        cerr << "err : bind addr " << strerror(errno) << endl;
        return -1;
    }

    // 2. 设置回调
    // 设置通用回调(没有特定的路径匹配)
    evhttp_set_gencb(http, http_req_gen_handler, NULL);
    // 设置特定路径的回调
    evhttp_set_cb(http, "/submit-form", http_req_hander_submit_form, NULL);

    event_base_loop(base, 0);

    event_base_dispatch(base);

    evhttp_free(http);

    return 0;
}

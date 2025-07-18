#include "event2/buffer.h"
#include <array>
#include <cstddef>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include "event2/event_compat.h"
#include "event2/event_struct.h"
#include "event2/keyvalq_struct.h"
#include "event2/util.h"
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstring>
#include <event2/util.h>
#include <fstream>
#include <iostream>
#include <event2/event.h>
#include <netinet/in.h>
#include <ostream>
#include <sys/socket.h>
#include <system_error>
using namespace std;

void http_post_resp_handler(struct evhttp_request *req, void *arg) {
}

void http_get_resp_handler(struct evhttp_request *req, void *arg) {
    cout << "http" << endl;

    if (req == NULL) {
        cout << "err : " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << endl;;
        return;
    }

    int code = evhttp_request_get_response_code(req);
    string code_line = evhttp_request_get_response_code_line(req);
    cout << code << " " << code_line << endl;

    string uri_s = evhttp_request_get_uri(req);
    cout << "uri : " << uri_s << endl;

    evhttp_uri *uri = evhttp_uri_parse(uri_s.c_str());
    string path = evhttp_uri_get_path(uri);
    if (path.empty() || path == "/")
        path = "/index.html";

    string file = path.substr(path.rfind('/') + 1);
    cout << "file : " << file << endl;

    ofstream out(file, ios::binary | ios::trunc);
    if (!out.is_open()) {
        cerr << system_category().message(errno) << endl;
        return ;
    }

    struct evbuffer *input = evhttp_request_get_input_buffer(req);
    char buf[1024] = {0};
    int len;
    while ((len = evbuffer_remove(input, buf, sizeof(buf) - 1)) > 0) {
        out.write(buf, len);
    }
}

void http_get_test(event_base *base) {
    // 1. 解析uri
    const char *uri_str = "http://127.0.0.1:8080/";
    evhttp_uri *uri = evhttp_uri_parse(uri_str);
    string host = evhttp_uri_get_host(uri);
    string path = evhttp_uri_get_path(uri);
    if (path.empty())
        path = "/";
    ev_uint16_t port = evhttp_uri_get_port(uri);
    string query = evhttp_uri_get_query(uri) == NULL ? "" : evhttp_uri_get_query(uri);
    string scheme = evhttp_uri_get_scheme(uri);

    cout << "scheme : " << scheme << endl;
    cout << "host : " << host << endl;
    cout << "path : " << path << endl;
    cout << "query : " << query << endl;
    cout << "port : " << port << endl;

    // 2. 准备请求
    // 2.1 准备连接
    struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    struct evhttp_connection *conn = evhttp_connection_base_bufferevent_new(base, NULL, bev, host.c_str(), port);

    // 2.2 准备请求
    struct evhttp_request *req = evhttp_request_new(http_get_resp_handler, NULL);

    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Host", host.c_str());

    // 3 发起请求
    evhttp_make_request(conn, req, EVHTTP_REQ_GET, uri_str);
}

void http_post_test(event_base *base) {
    // 1. 解析uri
    const char *uri_str = "http://127.0.0.1:8080/post.html";
    evhttp_uri *uri = evhttp_uri_parse(uri_str);
    string host = evhttp_uri_get_host(uri);
    string path = evhttp_uri_get_path(uri);
    if (path.empty())
        path = "/";
    ev_uint16_t port = evhttp_uri_get_port(uri);
    string query = evhttp_uri_get_query(uri) == NULL ? "" : evhttp_uri_get_query(uri);
    string scheme = evhttp_uri_get_scheme(uri);

    cout << "scheme : " << scheme << endl;
    cout << "host : " << host << endl;
    cout << "path : " << path << endl;
    cout << "query : " << query << endl;
    cout << "port : " << port << endl;

    // 2. 准备请求
    // 2.1 准备连接
    struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    struct evhttp_connection *conn = evhttp_connection_base_bufferevent_new(base, NULL, bev, host.c_str(), port);

    // 2.2 准备请求
    struct evhttp_request *req = evhttp_request_new(http_post_resp_handler, NULL);

    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Host", host.c_str());

    struct evbuffer *output = evhttp_request_get_output_buffer(req);
    evbuffer_add_printf(output, "aa=11&bb=22");

    // 3 发起请求
    evhttp_make_request(conn, req, EVHTTP_REQ_POST, uri_str);

}

int main (int argc, char *argv[]) {

    event_base *base = event_base_new();

    http_get_test(base);
    http_post_test(base);

    event_base_loop(base, 0);

    event_base_dispatch(base);


    return 0;
}

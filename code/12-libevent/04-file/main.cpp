#include "event2/buffer.h"
#include "event2/event_struct.h"
#include <bits/types/struct_timeval.h>
#include <cstdio>
#include <ostream>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <event2/event.h>
#include <system_error>
using namespace std;

#include <sys/inotify.h>

void read_file(evutil_socket_t fd, short flags, void *arg) {
    char buf[256];
    int cnt;
    if ((cnt = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[cnt] = 0;
        cout << buf ;
        flush(cout);
    }
    if (cnt < 0) {
        cerr << "read err" << error_code(errno, generic_category()).message() << endl;
    }
    if (cnt == 0) {
        cout << "read cnt == 0" << endl;
    }
}

// inotify 事件回调
void inotify_cb(evutil_socket_t fd, short flags, void *arg) {
    char buf[1024] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len = read(fd, buf, sizeof(buf));
    
    cout << __PRETTY_FUNCTION__ << endl;
    if (len < 0) return;
    
    const struct inotify_event *event = reinterpret_cast<struct inotify_event*>(buf);
    if (event->mask & IN_MODIFY) {
        // 调用文件读取函数
        cout << "文件修改" << endl;
    }
}

int main2() {
    // 创建 inotify 实例
    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        cerr << "inotify init failed" << endl;
        return -1;
    }

    // 添加监控
    int watch_fd = inotify_add_watch(inotify_fd, "./1.tmp", IN_MODIFY);
    if (watch_fd < 0) {
        cerr << "inotify watch failed" << endl;
        close(inotify_fd);
        return -1;
    }

    event_base *base = event_base_new();

    // 创建 libevent 事件
    event *ev = event_new(base, inotify_fd, EV_READ | EV_PERSIST, inotify_cb, NULL);
    event_add(ev, NULL);
    
    // 启动事件循环
    event_base_dispatch(base);

    // 清理资源
    inotify_rm_watch(inotify_fd, watch_fd);
    close(inotify_fd);
    return 0;
}

int main (int argc, char *argv[]) {

    return main2();

    event_config *config = event_config_new();
    event_config_require_features(config, EV_FEATURE_FDS);
    event_base *base = event_base_new_with_config(config);
    event_config_free(config);

    // event_base *base = event_base_new();

    int fd;
    if ((fd = open("./1.tmp", O_RDONLY)) < 0) {
        // error_code ec(errno, generic_category());
        // cerr << "failed to open 1.tmp : " << ec.message() << endl;
        cerr << "failed to open 1.tmp : " << 
            error_code(errno, generic_category()).message() << endl;
        return -1;
    }

    lseek(fd, 0, SEEK_END);

    // libevent 无法判断文件是否修改，对于为修改的文件也会一直调用read_file，因为未修改文件也是可读的。
    // 只有结合inotify使用
    event *ev = event_new(base, fd, EV_READ | EV_PERSIST | EV_ET, read_file , NULL);
    event_add(ev, NULL);
    event_base_dispatch(base);

    return 0;
}

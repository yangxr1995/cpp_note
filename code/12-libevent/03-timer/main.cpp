#include <bits/types/struct_timeval.h>
#include <iostream>
#include <event2/event.h>
#include <thread>
using namespace std;

void timer2_deal(evutil_socket_t fd, short flags, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;
    // this_thread::sleep_for(4s); // 默认单线程，会导致定时器处理延迟，但是超时触发不会被影响
}

void timer3_deal(evutil_socket_t fd, short flags, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;
}

void timer1_deal(evutil_socket_t fd, short flags, void *arg) {
    cout << __PRETTY_FUNCTION__ << endl;

    event * ev = (event *)arg;
    // if (false == evtimer_pending(ev, NULL)) {
    //     evtimer_del(ev);
    //     struct timeval tv = {1, 0};
    //     evtimer_add(ev, &tv);
    // }
}

int main (int argc, char *argv[]) {
    
    event_base *base = event_base_new();

    // 默认非持久化
    // #define evtimer_new(b, cb, arg)		event_new((b), -1, 0, (cb), (arg))
    event *ev_timer1 = evtimer_new(base, timer1_deal, event_self_cbarg());

    struct timeval tv1 = {1, 0};
    evtimer_add(ev_timer1, &tv1);

    // 持久化
    event *ev_timer2 = event_new(base, -1, EV_PERSIST, timer2_deal, event_self_cbarg());
    struct timeval tv2 = {2, 0};
    // 默认使用大根堆组织时间值
    evtimer_add(ev_timer2, &tv2); // 插入性能O(logn)

    {
        event *ev_timer3 = event_new(base, -1, EV_PERSIST, timer3_deal, NULL);
        timeval tv_in = {3, 0};
        // 双端链表将同样时间值的放到一起
        const timeval *tv3 = event_base_init_common_timeout(base, &tv_in);
        event_add(ev_timer3, tv3); // 插入性能O(1)
    }

    event_base_dispatch(base);

    return 0;
}

#include "event2/buffer.h"
#include "event2/event_struct.h"
#include <csignal>
#include <cstddef>
#include <iostream>
#include <event2/event.h>
#include <signal.h>
using namespace std;

void deal_sig_term(evutil_socket_t fd, short flags, void *arg)
{
    event * ev = (event *) arg;
    if (! event_pending(ev, EV_SIGNAL, NULL)) {
        cout << "事件未挂起，将事件置为pending状态" << endl;
        event_del(ev); // 如果事件是non-pending状态，则不会有任何动作
        event_add(ev, NULL);
    }
    cout << __PRETTY_FUNCTION__ << endl;
}

void deal_sig_int(evutil_socket_t fd, short flags, void *arg)
{
    cout << __PRETTY_FUNCTION__ << endl;
}

int main (int argc, char *argv[]) {

    event_enable_debug_mode();

    event_base *base = event_base_new();

	// event_new((b), (x), EV_SIGNAL|EV_PERSIST, (cb), (arg))
    event *esig_int = evsignal_new(base, SIGINT, deal_sig_int, NULL);
    if (esig_int == NULL) {
        cerr << "evsignal_new SIGINT err" << endl;
        return -1;
    }

    if (event_add(esig_int, NULL) != 0) {
        cerr << "event_add err" << endl;
        return -1;
    }

    // event *esig_term = event_new(base, SIGTERM, EV_SIGNAL, deal_sig_term, event_self_cbarg());
    event *esig_term = event_new(base, SIGTERM, EV_SIGNAL | EV_PERSIST, deal_sig_term, event_self_cbarg());
    if (event_add(esig_term, NULL) != 0) {
        cerr << "event_add SIGTERM err" << endl;
        return -1;
    }

    event_base_dispatch(base);
    event_free(esig_int);
    event_base_free(base);

    return 0;
}

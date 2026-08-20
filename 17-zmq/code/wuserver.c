#include "zhelpers.h"
#include "zmq.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//
// PUB - SUB
// 用于数据单向传输
//                         ┌────────┐
//                         │ 发布者 │
//                         ├────────┤
//                         │ PUB    │
//                         └────────┘
//                           绑定  
//                            │更新
//              ┌─────────────┼────────────────┐
//              │更新         │更新            │更新
//              ▼             ▼                ▼
//            连接           连接             连接
//          ┌────────┐     ┌────────┐       ┌────────┐
//          │ SUB    │     │ SUB    │       │ SUB    │
//          ├────────┤     ├────────┤       ├────────┤
//          │ 订阅者 │     │ 订阅者 │       │ 订阅者 │
//          └────────┘     └────────┘       └────────┘

int main(int argc, char *argv[]) {

    void *zmq_ctx = zmq_ctx_new();
    void *zmq_puber = zmq_socket(zmq_ctx, ZMQ_PUB);
    int rc = zmq_bind(zmq_puber, "tcp://*:5556");
    assert(rc == 0);
    rc = zmq_bind(zmq_puber, "ipc://weather.ipc");
    assert(rc == 0);

    srandom((unsigned)time(NULL));
    while (1) {
        int zipcode, temperature, relhumidty;
        zipcode = randof(100000);
        temperature = randof(215) - 80;
        relhumidty = randof(50) + 10;

        char update[20];
        snprintf(update, sizeof(update) - 1, "%05d %d %d", zipcode, temperature, relhumidty);
        // 不论是否有 订阅者连接都会不停的发, 导致订阅者会错过消息
        s_send(zmq_puber, update);

        printf("zipcode[%d] temperature[%d] relhumidity[%d]\n",
                zipcode, temperature, relhumidty);
    }

    zmq_close(zmq_puber);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}

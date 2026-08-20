//
//                         ┌───────┐
//                         │ 发生器│
//                         ├───────┤
//                         │ PUSH  │
//                         └───────┘
//                           任务
//                            │
//              ┌─────────────┼──────────────┐
//              │             │              │
//              ▼ 任务        ▼ 任务         ▼ 任务 
//          ┌──────┐       ┌──────┐       ┌──────┐
//          │ PULL │       │ PULL │       │ PULL │
//          ├──────┤       ├──────┤       ├──────┤
//          │ 工人 │       │ 工人 │       │ 工人 │
//          ├──────┤       ├──────┤       ├──────┤
//          │ PUSH │       │ PUSH │       │ PUSH │
//          └───┬──┘       └──┬───┘       └──┬───┘
//              │ 结果        │ 结果         │ 结果
//              └─────────────┼──────────────┘
//                            │
//                            ▼
//                         ┌───────┐
//                         │ PULL  │
//                         ├───────┤
//                         │ 接收器│
//                         └───────┘
//
// 实际上是 PUSH 做server ， PULL 发起连接
// 任务发生器
#include "zmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <zhelpers.h>

int main(int argc, char *argv[])
{
    void *zmq_ctx = zmq_ctx_new();
    void *sender = zmq_socket(zmq_ctx, ZMQ_PUSH);
    zmq_bind(sender, "tcp://*:5557");

    void *sink = zmq_socket(zmq_ctx, ZMQ_PUSH);
    zmq_connect(sink, "tcp://localhost:5558");
    printf("Press Enter when the workers are ready:");
    getchar();

    printf("Sending tasks to workers...\n");
    s_send(sink, "0"); // 任务开始
    zmq_close(sink);
    printf("Create tasks to workers...\n");

    srandom((unsigned)time(NULL));

    int task_nb;
    int total_msec = 0;
    printf("\n");
    for (task_nb = 0; task_nb < 1000; task_nb++) {
        int workload;
        // 随机工作负荷
        workload = randof(100) + 1;
        total_msec += workload;
        char string[10];
        sprintf(string, "%d", workload);
        printf("s");
        fflush(stdout);
        s_send(sender, string);
        printf(".");
        fflush(stdout);
    }
    printf("\n");
    printf("Total expected cost : %d msec\n", total_msec);
    sleep(1);

    zmq_close(sender);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}



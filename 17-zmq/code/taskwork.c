// 并行任务工人

#include "zmq.h"
#include <stdio.h>
#include <stdlib.h>
#include <zhelpers.h>

int main(int argc, char *argv[])
{
    void *zmq_ctx = zmq_ctx_new();

    // 用于接受任务
    void *receiver = zmq_socket(zmq_ctx, ZMQ_PULL);
    zmq_connect(receiver, "tcp://localhost:5557");

    // 用于发送结果
    void *sender = zmq_socket(zmq_ctx, ZMQ_PUSH);
    zmq_connect(sender, "tcp://localhost:5558");

    while (1) {
        char *string = s_recv(receiver);
        fflush(stdout);
        printf("%s.", string);

        s_sleep(atoi(string));
        free(string);

        // 发送给接收器
        s_send(sender, "");
    }

    zmq_close(receiver);
    zmq_close(sender);
    zmq_ctx_destroy(zmq_ctx);
    return 0;
}


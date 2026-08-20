// 任务接收器
#include "zmq.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <zhelpers.h>

int main(int argc, char *argv[])
{
    void *zmq_ctx = zmq_ctx_new();
    void *receiver = zmq_socket(zmq_ctx, ZMQ_PULL);
    zmq_bind(receiver, "tcp://*:5558");

    char *string = s_recv(receiver);
    free(string);

    int64_t start_time = s_clock();

    int task_nb;
    for (task_nb = 0; task_nb < 100; task_nb++) {
        char *string = s_recv(receiver);
        free(string);
        if ((task_nb / 10) * 10 == task_nb) {
            printf(":");
        }
        else {
            printf(".");
        }
        fflush(stdout);
    }
    printf("Total elapsed time: %d msec\n",
            (int)(s_clock() - start_time));
    fflush(stdout);

    zmq_close(receiver);
    zmq_ctx_destroy(zmq_ctx);

    return 0;
}

#include "zmq.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zhelpers.h>

int main(int argc, char *argv[])
{
    void *zmq_ctx = zmq_ctx_new();

    printf("Collecting updates from weather server...\n");
    void *zmq_suber = zmq_socket(zmq_ctx, ZMQ_SUB);
    int rc = zmq_connect(zmq_suber, "tcp://localhost:5556");
    assert(rc == 0);

    // 只接受 msg 首部字节为 "10001" (不包含\0) 的消息
    char *filter = (argc > 1) ? argv[1] : "10001";
    rc = zmq_setsockopt(zmq_suber, ZMQ_SUBSCRIBE, filter, strlen(filter));
    assert(rc == 0);

    int update_nb;
    long total_temp = 0;
    for (update_nb = 0; update_nb < 100; update_nb++) {
        char *string = s_recv(zmq_suber);
        int zipcode, temperature, relhumidity;
        sscanf(string, "%d %d %d",
                &zipcode, &temperature, &relhumidity);
        total_temp += temperature;
        free(string);
        printf("zipcode[%d] temperature[%d] relhumidity[%d]\n",
                zipcode, temperature, relhumidity);
    }

    printf("Avaerage temperature for zipcode '%s' was %dF\n",
            filter, (int)(total_temp / update_nb));

    zmq_close(zmq_suber);
    zmq_ctx_destroy(zmq_ctx);
    
    return 0;
}

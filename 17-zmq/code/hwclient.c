//
//  Hello World 客户端
//  连接REQ套接字至 tcp://localhost:5555
//  发送Hello给服务端，并接收World
//
//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//编译：gcc -o hwclient hwclient.c -lzmq
int main (void)
{
    printf ("Connecting to hello world server...\n");
    void *context = zmq_ctx_new ();
    //  连接至服务端的套接字
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    int request_nbr;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [10];
        printf ("A 正在发送 Hello %d...\n", request_nbr);
        zmq_send (requester, "Hello", 5, 0);
       //zmq_recv (requester, buffer, 5, 0);        // 收到响应才能再发
        //printf ("接收到 Hello World %d\n", request_nbr);
        printf ("B 正在发送 Darren %d...\n", request_nbr);
        zmq_send (requester, "Darren", 6, 0);   // 无效
        zmq_send (requester, "king", 4, 0);      // 无效
        zmq_recv (requester, buffer, 6, 0);        // 收到响应才能再发
        printf ("接收到Darren World %d\n", request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}
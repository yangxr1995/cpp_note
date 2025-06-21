# 源代码和汇编混合显示

在Linux下发生一个进程死机问题，由于进程跑在客户的机器上，没有启用Coredump，因此死机没有CoreDump文件，仅有进程临死前的调用栈信息；调用栈中保存了导致死机的函数的入参和局部变量的内容，需要对其进行解析，确定临死前入参和局部变量的内容。

（1）入参的解析 : 如函数A调用函数B，B的入参可以这样确定：首先在堆栈中找到A的EIP，EIP的前一个地址就是A的EBP，EIP的后一个地址是B的第一个入参，依次类推可以得到B的所有入参内容。

（2）局部变量的解析，利用objdump进行反汇编，但是由于很多人对汇编语句不是很熟悉，因此需要将C语言和其对应的汇编结合起来看，举例如下
objdump -j .text --no-show-raw-insn -Sl --prefix=/cygdrive/f/Workspace/CPP/CSC/CSC8.2 --prefix-strip=10 --start-address=0x83e5da0 cscf.scu.dbg | more

-j 后面是段的类型，有.text, .data，.bss等；

--no-show-raw-insn，不显示机器指令；

-S，尽可能显示源码；

-l，显示行号；

--prefix，源程序在本地的目录前缀；

--prefix-strip，如果编译时的目录前缀与现在不同的话，需要对原来的前缀进行修改，10代表去除原前缀的目录层次数，该选项必须与--prefix同时使用；

--start-address，代表某函数的额入口地址，可以通过gdb的info line XXXX得到，XXXX是函数名；

cscf.scu.dbg，是带调试信息的可执行文件；

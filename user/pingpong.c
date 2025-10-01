// pingpong.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // 与题目无关，只是想测试一下，父进程修改普通变量后子进程是否同步修改
    int i = 100;
    // 创建两个管道描述符，前者用于父进程向子进程读和写数据
    // 后者，用于子进程向父进程读和写数据
    int fd_p2c[2], fd_c2p[2];

    // 调用pipe()系统函数创建两个管道
    pipe(fd_p2c);
    pipe(fd_c2p);

    if (fork() == 0) {
        sleep(10);
        // 打印当前 pid、i 的地址和值，以及两个管道地址，fork 会复制父进程的文件描述符表，所以父子进程打印的地址是相同的
        printf("here's child process, pid = %d, i = %d, i addr is %pn, fd_p2c addr is %pn, fd_c2p addr is %pn \n\n", getpid(), i, &i, &fd_p2c, &fd_c2p);

        // 父进程向子进程发送消息
        int port_read = fd_p2c[0];
        // 子进程向父进程写数据
        int port_write = fd_c2p[1];

        char content_receive[1024] = {0};
        char content_send[1024] = {"daddy!\n"};

        // 读取父进程发送的消息，默认阻塞调用
        read(port_read, content_receive, sizeof(content_receive));
        printf("child received: %s", content_receive);
        printf("%d: received ping\n\n", getpid());

        // 写入消息至管道，回复父进程
        write(port_write, content_send, sizeof(content_send));

        sleep(10);
        
        exit(0);
    } else {
        // 不为 0 则表示是父进程的代码
        i = 101;
        // pn应该就是指针吧？
        printf("here's parent process, pid = %d, i = %d, i addr is %pn, fd_p2c addr is %pn, fd_c2p addr is %pn \n", getpid(), i, &i, &fd_p2c, &fd_c2p);

        // 子进程向父进程写的，父进程读
        int port_read = fd_c2p[0];
        int port_write = fd_p2c[1];

        char content_receive[1024] = {0};
        char content_send[1024] = {"call me daddy please \n"};

        // 题目要求父进程先发送消息
        write(port_write, content_send, sizeof(content_send));

        // 发完消息等待子进程回复
        read(port_read, content_receive, sizeof(content_receive));
        printf("parent received: %s", content_receive);
        printf("%d: received pong\n\n", getpid());

        sleep(10);

        exit(0);
    }
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void exec_by_child(char *program, char **args) {
    // fork 出一个子进程去执行
    if (fork() == 0) { // 子进程
        // program：echo
        // args：echo xxx yyy
        // 实际exec读取参数的时候会pass掉args的echo，这个一般是作为程序名
        if (exec(program, args) == -1) {
            // 这里的失败是通过打印信息到控制台，无论如何都返回exit(0)，让主进程顺利结束
            printf("\nexec faild, not found the %s!\n\n", program);
        }
        exit(0);
    }
    return;
}

int main(int argc, char *argv[]) {
    // 存放从 stdin 读入的原始字节，
    char buf[1024];                           // 指令缓冲区
    char *start_ptr = buf, *last_ptr = buf;   // 参数列表的开始、结束指针
    // 存放 exec 用的 argv 数组（指针数组）
    char *args_buf[128];                      // 参数缓冲区
    char **args = args_buf;                   // 当前参数的指针

    if (argc < 2) {
        // xargs echo bye
        // argc：3
        // argv：
        // argv[0] "xargs" （程序名）
        // argv[1] "echo" （要执行的命令）
        // argv[2] "bye" （命令的固定参数）
        // argv[3] NULL
        fprintf(2, "Usage: <command> <params> | xargs <command>  <params>\n");
        exit(1);
    }
    // 我理解的是xargs是现在编写的程序，
    // 然后管道前面的部分其实属于别的程序的输出，将这个输出作为xargs的参数

    // 读取所有传入的参数到args
    // xargs echo bye中echo bye的参数
    for (int i = 1; i < argc; i++) {
        *args = argv[i];
        args++;
    }

    // cur_args指向已有args的下一位
    // 用于拼接从标准管道流入的参数
    char **cur_arg = args;

    // 循环读入参数
    // 逐字节读取
    // buf是从标准输入管道读取数据
    while (read(0, start_ptr, 1) != 0) {

        // 遇到空格或换行符，则将参数结束符置为空，并记录参数的结束位置
        if (*start_ptr == ' ' || *start_ptr == '\n') {
            *start_ptr = '\0';          // 将当前字符替换为字符串结束符
            // *start_ptr置为结束 last_ptr就表示上一个参数
            *(cur_arg++) = last_ptr;    // 将当前参数的起始地址存入参数列表
            last_ptr = start_ptr + 1;   // 更新last_ptr指向下一个字符

            // 如果读取到换行符 则执行子进程
            if (*start_ptr == '\n') {
                // 参数的结束位置
                *cur_arg = 0;
                exec_by_child(argv[1], args_buf);
                // 重新读取参数
                cur_arg = args;
            }
        }
        start_ptr++;
    }

    // 如果还有未处理的参数，同样执行子进程
    if (cur_arg != args) {
        // 把当前位置置空
        *start_ptr = '\0';
        // last_ptr每次记录的是最近一个参数
        *(cur_arg++) = last_ptr;
        *cur_arg = 0;
        // 把输出的结果给
        exec_by_child(argv[1], args_buf);
    }

    // 等待子进程处理完再结束父进程
    // wait(0)等待任意一个子进程结束
    // 返回对应的子进程的PID
    // 如果没有子进程了就会返回-1
    while (wait(0) != -1) {
        // wait for child processes
    }
    exit(0);
}
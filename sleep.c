// sleep.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int tick;
    // 第一个参数默认是程序名，
    // 第二个参数开始才是实际参数
    // 这边保证sleep传参只有一个，表示休眠的ticks数量
    if (argc != 2) {
        fprintf(2, "sleep: usage: sleep <tick>\n");
    }
    // The command-line argument is passed as a string; you can convert it to an integer using atoi
    tick = atoi(argv[1]);
    // Use the system call sleep
    sleep(tick);
    // Make sure main calls exit() in order to exit your program.
    exit(0);
}

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

/* retrieve the filename from whole path */
char *basename(char *pathname) {
  char *prev = 0;
    // 每次用 strchr 从某一个地址开始逐步寻找 '/'
  char *curr = strchr(pathname, '/');
  while (curr != 0) {
    prev = curr;
    curr = strchr(curr + 1, '/');
  }
    // 返回 /【文件名】
  return prev;
}

/* recursive */
void find(char *curr_path, char *target) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
    // O开头，用于向 open() 系统调用传递标志（flags）
    // O_RDONLY：read only
  if ((fd = open(curr_path, O_RDONLY)) < 0) {
      // 2标准错误
    fprintf(2, "find: cannot open %s\n", curr_path);
    return;
  }
// 判断是文件还是目录 st.type
  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", curr_path);
    close(fd);
    return;
  }

  switch (st.type) {
// 若是普通文件：取出其“最后一段文件名”与 target 比较；
  case T_FILE:{
    char *f_name = basename(curr_path);
    int match = 1;
    // strcmp比较两个字符串的内容是否相同
    if (f_name == 0 || strcmp(f_name + 1, target) != 0) {
      match = 0;
    }
    if (match)
      printf("%s\n", curr_path);
    close(fd);
    break;
  }
// 若是目录：读取该目录的每个目录项 de，拼出“下一层的完整路径”
  case T_DIR:{
    // 现在访问的是当前路径
    // 构建下一个层级的路径字符串
    memset(buf, 0, sizeof(buf));
    uint curr_path_len = strlen(curr_path);
    memcpy(buf, curr_path, curr_path_len);
    buf[curr_path_len] = '/';
    // 指针移动到/后的位置，接下来要写入子文件名的地方
    p = buf + curr_path_len + 1;
    // 从目录 fd 中读取一个个目录项（struct dirent）
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        // de.inum == 0：表示该目录项未使用（空槽）。
        // . 和 ..：当前目录和上级目录，跳过它们。
      if (de.inum == 0 || strcmp(de.name, ".") == 0 ||
          strcmp(de.name, "..") == 0)
        continue;
        // 将当前目录项的文件名拼接在路径后面
        // 其实也不一定是文件名，仍然可能还是路径名
        // DIRSIZ指的是文件名/路径名长度的最大值
        // 这里用了 DIRSIZ 固定长度复制，
        // 即使文件名较短也会复制多余字节，
        // 所以必须显式加 \0 来确保字符串正确终止。
      memcpy(p, de.name, DIRSIZ);
        // 0放在结尾表示结束
      p[DIRSIZ] = 0;
      find(buf, target); // recurse
    }
    close(fd);
    break;
  }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(2, "usage: find [directory] [target filename]\n");
    exit(1);
  }
// 初始路径和目标文件名
  find(argv[1], argv[2]);
  exit(0);
}

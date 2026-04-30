#include <stdio.h>

int main() {
    char name[100];
    printf("请输入你的名字: ");
    scanf("%99s", name);
    printf("你好，%s！\n", name);
    return 0;
}

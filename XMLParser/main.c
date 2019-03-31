#include <stdio.h>

char *matchXMLClass(const char *ipAddr) {
    char path[] = "\\ipList\\ipClass.xml";
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return "Failure";
    }
    char buff[50];
    while (fgets(buff, 50, fp))
    {
        if (strstr(buff, "<class>") != NULL) {

        }
    }
    getchar();
}

int main() {
    printf("Hello, World!\n");
    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <ctype.h>

char *trim(char *str) {
    int start, end, i;
    if (str) {
        for (start = 0; isspace(str[start]); start++) {}
        for (end = (int) (strlen(str) - 1); isspace(str[end]); end--) {}
        for (i = start; i <= end; i++) {
            str[i - start] = str[i];
        }
        str[end - start + 1] = '\0';
        return (str);
    } else {
        return NULL;
    }
}

int matchXMLList(const char *listType, const char *ipAddr);

int searchTag(FILE *fpTag, char *tagName, const char *ipAddr) {
    char buff[50];
    char endTag[10];
    strcat(endTag, "</");
    strcat(endTag, tagName);
    strcat(endTag, ">");
    while (fgets(buff, 50, fpTag)) {
        if (strstr(buff, endTag) == NULL) {
            if (strstr(buff, ipAddr) != NULL) {
                return 0;
            }
        } else {
            return -1;
        }
    }
    return -1;
}

char *matchXMLClass(const char *ipAddr, char *result) {
    char path[_MAX_PATH];
    char fileLoc[] = "\\ipList\\ipClass.xml";
    _getcwd(path, _MAX_PATH);
    strcat(path, fileLoc);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return "Failed";
    }
    FILE *fpTag, *fpSearch;
    char buff[50];
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, "<class>") != NULL) {
            fpTag = fp;
            fpSearch = fp;
            if (searchTag(fpTag, "class", ipAddr) == 0) {
                // Get match in xml
                while (fgets(buff, 50, fpSearch)) {
                    if (strstr(buff, "<description>") != NULL) {
                        fgets(result, 100, fpSearch);
                        trim(result);
                        trim(result);
                        return result;
                    }
                }
            }
        }
    }
    return "Failed";
}

int matchXMLList(const char *listType, const char *ipAddr) {
    char path[_MAX_PATH];
    char fileLoc[] = "\\ipList\\";
    _getcwd(path, _MAX_PATH);
    strcat(path, fileLoc);
    strcat(path, "ip");
    strcat(path, listType);
    strcat(path, ".xml");
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Open xml file failed.\n");
        return -1;
    }
    FILE *fpTag, *fpSearch;
    char buff[50];
    while (fgets(buff, 50, fp)) {
        if (strstr(buff, "<ip>") != NULL) {
            fpTag = fp;
            fpSearch = fp;
            if (searchTag(fpTag, "ip", ipAddr) == 0) {
                // Get match in xml
                while (fgets(buff, 50, fpSearch)) {
                    if (strstr(buff, "<enable>") != NULL) {
                        fgets(buff, 50, fpSearch);
                        return atoi(buff);
                    }
                }
            }
        }
    }
}

int main() {
    char result[100];
    const char *ip = "127.0.0.1";
    printf("%s\n", matchXMLClass(ip, result));
    printf("%d\n", matchXMLList("BlackList", ip));
    printf("%d\n", matchXMLList("WhiteList", ip));
    return 0;
}
#include "client.cpp"

int main() {
    char msg[1024];
    strcpy(msg, "caonima");
    //msg_encryption(msg);
    //printf(result);
    strcpy(msg, "813DB7B34102D5E2");
    msg_decryption(msg);
    printf(msg_result);
    return 0;
}
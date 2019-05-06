//
// Created by rainy on 5/7/2019.
//

#ifndef CHAT_CLIENT_CLIENT_H
#define CHAT_CLIENT_CLIENT_H


// char userName[16] = {0};

unsigned __stdcall ThreadRecv(void *param);

unsigned __stdcall ThreadSend(void *param);

int msg_encryption(char* msg_text);

int msg_decryption(char* msg_text);

// int genKey();

int sayHello(SOCKET ClientSocket);

int handShaking(SOCKET ClientSocket);

int ConnectChatServer();

int main();


#endif //CHAT_CLIENT_CLIENT_H

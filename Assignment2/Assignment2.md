# Assignment Description and Requirements

Students are required to write an Internet Secure Chat Program by using Windows Socket Programming C++. The program should ensure that the users can chat securely on the Internet (i.e., User A can send encrypted messages to User B; and vice versa).
The sample programs (namely, “server.cpp” and “client.cpp”) can be downloaded from the Web site. The illustration of the actual operations is as follows:
 - Step 1: User A establishes a TCP connection with User B.
 - Step 2: User A sends his/her user name to User B.
 - Step 3: User B accepts the request, and sends a confirmation to User A.
 - Step 4: After identity verification, User A and B start to exchange the encrypted
 - messages by using their predefined key(s). All the conversations (decrypted messages) should be recorded in a log file.
 - Step 6: To terminate the chat, both sides can use a predefined command to terminate the communications.

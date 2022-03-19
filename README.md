#Winter 2022 CIS 427 Project 2: Client and server (select() function)
##Collaborators: Maya Soleimani
This is a programn is an implemented online address using TCP sockets that is able to be accessed by multiple clients to communicate to a server.

Commands *ADD- Able to add a first name, last name, and phone number to .txt file. *DELETE- Able to remove added contact in address book. *LIST - Able to list contacts that are input by client *SHUTDOWN- Able to have client shut down both server and client *QUIT- when commanded on, closes client first, and server disconnects from client.

#UPDATE FOR PROGRAM 2:
IDE: Visual Studios 2019
Compiler used: gcc.exe (Rev5, Built by MSYS2 project) 10.3.0

*was not able to get files to compile together, I can run one (mainSource.cpp) on putty, but none of the commands are implemented.
*attempted to have  TCPListen.cpp, TCPListen.h,and main.cpp working together in a *class* data structure, ended up having too many accessibility issues for time permitted.
*when trying to run on UMD server, timeout occurs after about 10 seconds, otherwise on 127.0.0.1, built and could communicate between multiple putty windows

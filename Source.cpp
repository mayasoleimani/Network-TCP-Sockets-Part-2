#include<iostream>
#include<WS2tcpip.h>	
#include<string>
#define SERVER_PORT 2955

#pragma comment (lib, "ws2_32.lib")
using namespace std;


//count  of addresses
int gettingcount(SOCKET sock) {

	
		char buf[4096];
		string hold;
		int bytesRec = recv(sock, buf, 4096, 0);
		if (bytesRec > 0) //message exists, character length greater than 0
		{
			//if add,del,shutdown,quit,list
			hold = string(buf, 0, bytesRec);

			return stoi(hold);
		}


}


int main()
{
	int count = 1000;
	string ip = "127.0.0.1"; //server ip, standard test ip

	//intialize Winsock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int iRes = WSAStartup(ver, &data);
	if (iRes != 0)
	{
		cerr << "Unable to start Winsock, Error:" << iRes << endl;
		return 0;
	}
	//create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Unable to create socket, Error:" << WSAGetLastError() << endl;
		WSACleanup();
		return 0;
	}

	//fill in hint structure
	sockaddr_in sin; //buffer
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, ip.c_str(), &sin.sin_addr);
	//connect to a server
	int connRes = connect(sock, (sockaddr*)&sin, sizeof(sin));
	if (connRes == SOCKET_ERROR)
	{
		cerr << "Unable to connect to server, Error:" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 0;

	}
	//loop to send and recieve datas

	char buf[4096];
	string userInfo; //user input

	Yield(2000);
	count = gettingcount(sock);


	//user input
	do {
		cout << "client: ";
		getline(cin, userInfo);

		if (userInfo.size() > 0)
		{
		
			//sending message once verified
			int mesRes = send(sock, userInfo.c_str(), userInfo.size() + 1, 0);
			if (mesRes != SOCKET_ERROR)
			{
			
				int bytesRec = recv(sock, buf, 4096, 0);
				if (bytesRec > 0) //message exists, character length greater than 0
				{
					//if add,del,shutdown,quit,list

					string commWord = string(buf, 0, bytesRec);
					string commMsg = string(buf, 0, bytesRec);

					for (int i = 0; i < commMsg.size() - 1; i++)
					{
						if (commMsg.at(i) == ' ')
						{
							commWord = commMsg.substr(0, i);
							commMsg = commMsg.substr(i + 1); //second part of message after command
							break;
						}
					}
					if (commWord == "ADD")
					{
						++count;
					}
					else if (commWord == "LIST")
					{
					}
					else if (commWord == "LOGIN")
					{
					}
					else if (commWord == "QUIT")
					{
						closesocket(sock);
						goto quitting;

						exit(0);
						return 0;
					}
					else if (commWord == "SHUTDOWN")
					{
						return -1;
					}
					
				}
			}
		}

	} while (userInfo.size() > 0);
	//shut down
	quitting:
	closesocket(sock);
	WSACleanup(); //shuts down winsock
	return 0;

}
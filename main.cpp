#include<iostream>
#include<fstream>
#include<WS2tcpip.h>	
#include <iomanip>
#include<string>
#include <thread>
#define SERVER_PORT 2955
#define SERVER_PORT2 2954
#pragma comment (lib, "ws2_32.lib")
//raw connect

using namespace std;

fstream filein;
fstream output;
fstream login;
string filter;
string lastaddress;
int addtracker = 1000;

//users in
bool loggedincheck[4] = {false,false,false,false};


int client1(int count);
int client2(int count);


int main()
{
	int count = 1000;

	
	output.open("Book.txt", ios::out | ios::in);
	
	
	if (!(output.peek() == EOF)) {
		getline(output, lastaddress);
		while (output.good()) {

			filter = lastaddress;
			getline(output, lastaddress);


		}
		for (int i = 0; i < filter.size(); i++)
		{
			if (filter.at(i) != '\0')
			{
				lastaddress = filter.substr(i, 4);
				break;
			}
		}
			count = stoi(lastaddress);
			addtracker = count;
	}
	output.close();

	thread first(client1,count);
	thread second(client2, count);



	first.join();
	second.join();

	return 0;
}


int client1(int count) {
	int checkloginuser = 0;
	bool loggedin = false;
	while (true)
	{


		//initialize winsock
		WSADATA winsData;
		WORD verReq = MAKEWORD(2, 2); //to request which version we want, 2.2

		int wsOk = WSAStartup(verReq, &winsData);
		if (wsOk != 0)
		{
			cerr << "Error initializing winsock. Try again. \n";
			return 0;
		}

		//create socket
		SOCKET listening = socket(PF_INET, SOCK_STREAM, 0); //takes address family, opens tcp socket, listening is our socket
		if (listening == INVALID_SOCKET)
		{
			cerr << "Socket creation failed. Try again. \n";
			return 0;
		}
		//                     more error handling?

		//bind socket to an ip address and port
		sockaddr_in sin; //fill in a hint structure//indicates preferred socket/protocol 
		sin.sin_family = PF_INET; //using version 4
		sin.sin_port = htons(SERVER_PORT); //htons=host to network short
		sin.sin_addr.S_un.S_addr = INADDR_ANY; //to bind to any address

		bind(listening, (sockaddr*)&sin, sizeof(sin)); //binds to networking code, file descriptor

		//tell winsock , the socket is listening
		listen(listening, SOMAXCONN);
		//wait for a connection

		sockaddr_in client; //input address for client
		int clientSize = sizeof(client);

		SOCKET new_s = accept(listening, (sockaddr*)&client, &clientSize);
		if (new_s == INVALID_SOCKET)
		{
			cerr << "Invalid socket. Try again. \n";
			return 0;

		}


		char host[NI_MAXHOST]; //clients remote name
		char service[NI_MAXHOST]; // port the client connects to, service buffer

		memset(host, 0, NI_MAXHOST);  //setting to zero
		memset(service, 0, NI_MAXHOST);

		// check if we can get name info, if not, rely on what we have
		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			cerr << host << "Connected on port" << service << endl;
		}
		else
		{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cerr << host << " connected on port" << ntohs(client.sin_port) << endl;
		}
		//close listening socket
		closesocket(listening);

		//accept message from client

		char buf[4096]; //mem for buf message
		send(new_s, to_string(count).c_str(), 4096, 0);

		while (true)
		{
			memset(buf, 0, 4096);

			//wait for client to send data
			int bytesRec = recv(new_s, buf, 4096, 0);
			if (bytesRec == SOCKET_ERROR)
			{
				cerr << "Error recieving. Try again. \n";
				break;
			}
			if (bytesRec == 0)
			{
				cerr << "Disconnected from client. \n"; //when client disconnects, or quits
				break;
			}

			// address book editing

			//command word being sent (add,delete,list,shutdown,quit)
			string commWord;
			//command message being sent
			string commMsg = string(buf, 0, bytesRec);
			//cerr << commMsg << endl;
			for (int i = 0; i < commMsg.size() - 1; i++)
			{
				if (commMsg.at(i) == ' ')
				{
					commWord = commMsg.substr(0, i); //command
					commMsg = commMsg.substr(i + 1); //second part of message after command
					break;
				}

			}

			if (loggedin)
			{

				if (commWord == "ADD")
				{
					output.open("Book.txt", ios::out | ios::in);
					//puts output to end of file
					output.seekg(output.tellg(), ios::end);
					//output count 
					output << ++addtracker << " " << left << setw(25) << commMsg << "\n";
					output.close();
					goto logoutvalid;
				}
				else if (commMsg == "SHUTDOWN") //comword orig
				{
					//checkloginuser
					if (loggedincheck[0]==true) {
						closesocket(new_s);
						WSACleanup();
						exit(0); 
						break;
					}
					else 
					{
						cout << "402 User not allowed to execute this command";
						output.seekg(output.tellg(), ios::beg);
					}
				}
				else if (commWord == "DELETE")
				{
					output.open("Book.txt", ios::out | ios::in);
					string book;
					string blank = "                             "; //replaces deleted message
					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					output >> book;
					int pos = output.tellg();

					while (output.good())
					{

						if (commMsg == book)
						{

							output.seekg(output.tellg(), ios::beg); //resets to where we currently are
							output.seekg(pos);


							output << blank << endl; //deletes
							break;
						}

						output >> book;
						pos = output.tellg();

						for (int i = 0; i < book.size() - 1; ++i)
						{
							if (isalnum(book.at(i))) // if character is true
							{
								book = book.substr(i);
								break;
							}
						}


					}

					if (!output.good())
					{
						cout << "Invalid address, try again." << endl;
						output.clear(); //sets bit to good
						output.seekg(output.tellg(), ios::beg);

					}

					// search for address
					output.seekg(32);
					// go back an int bit
					output.close();
					goto logoutvalid;
				}
				else if (commMsg == "LOGOUT")
				{

					loggedincheck[checkloginuser] = false;
					loggedin = false;
					cout << "200 OK" << endl;
					goto logoutvalid;

				}

			}

			if (commMsg == "LIST")
			{
				output.open("Book.txt", ios::out | ios::in);
				cout << "200 OK" << endl;
				string book;
				output.clear();
				//moves pointer back to beginning of file
				output.seekg(output.tellg(), ios::beg);
				output.seekg(0, ios::beg);
				output.clear();
				getline(output, book);

				//while good, keep printing
				while (output.good())
				{
					cout << book << endl;
					getline(output, book);
				}
				cout << book << endl;
				//clears error place in output
				output.clear();
				output.close();

			}
			else if (commWord == "LOGIN") {
				if (!loggedin) {
					login.open("login.txt", ios::out | ios::in);
					string userlogin;
					login.clear();
					//moves pointer back to beginning of file
					login.seekg(output.tellg(), ios::beg);
					login.seekg(0, ios::beg);
					login.clear();
					getline(login, userlogin);
					int pos = login.tellg();
					int usernum = 0;
					while (login.good()) {

						// if login doesnt match
						if (commMsg.size() == userlogin.size())
						{

							for (int i = 0; i < userlogin.size() - 1; ++i)
							{
								if (commMsg.at(i) != userlogin.at(i)) {
									goto skip;
								}

							}
							loggedincheck[usernum] = true;
							checkloginuser = usernum;
							loggedin = true;
							break;
						}
					skip:
						usernum++;
						// grabs entire line, causes good() to fail
						getline(login, userlogin);



					}
					if (loggedincheck[checkloginuser])
					{
						cout << "200 OK" << endl;
					}
					else
					{
						cout << "410 Wrong UserID and Password" << endl;
					}
					login.close();
				}

			}
			else if (commMsg == "WHO") {
				cout << "200 OK" << endl << "The list of active users:" << endl;
				if (loggedincheck[0])
					cout << "root" << "  127.0.0.1 \n";
				if (loggedincheck[1])
					cout << "john" << "  127.0.0.1 \n";
				if (loggedincheck[2])
					cout << "david" << "  127.0.0.1 \n";
				if (loggedincheck[3])
					cout << "mary" << "  127.0.0.1 \n";

			}
			else if (commMsg == "QUIT")
			{
				loggedincheck[checkloginuser] = false;
				loggedin = false;
				cout << "200 OK" << endl;
				cout << "Client now closed." << endl;
				goto close;
				//closes just client

			}
			else if (commWord == "LOOKUP") 
			{
				output.open("Book.txt", ios::out | ios::in);
				if (commMsg.at(0) == '1') 
				{

					string firstname;
					string lastname;
					string phonenumber;
					string addressnum;
					string book;
					commMsg = commMsg.substr(2);

					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					getline(output, book);
					int pos = output.tellg();



					while (output.good())
					{
						for (int i = 0; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								addressnum = book.substr(0, i);
								break;
							}
						}//setting first name
						for (int i = addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
								break;
							}
						} //setting last name
						for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
								break;
							}
						} //setting phone number
						for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
						{
							if (book.at(i) == ' ') {
								phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
								break;
							}
						}

						if (firstname == commMsg) {
							cout << "Found match" << endl;
							cout << book << endl;
						}
				


						getline(output, book);


					}
				}
				else if (commMsg.at(0) == '2') 
				{

					string firstname;
					string lastname;
					string phonenumber;
					string addressnum;
					string book;
					commMsg = commMsg.substr(2);

					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					getline(output, book);
					int pos = output.tellg();



					while (output.good())
					{
						for (int i = 0; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								addressnum = book.substr(0, i);
								break;
							}
						}//setting first name
						for (int i = addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
								break;
							}
						} //setting last name
						for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
								break;
							}
						} //setting phone number
						for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
						{
							if (book.at(i) == ' ') {
								phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
								break;
							}
						}

						if (lastname == commMsg)
							cout << "Found match" << endl;
							cout << book << endl;


						getline(output, book);


					}
				}
			else if (commMsg.at(0) == '3') {

				string firstname;
				string lastname;
				string phonenumber;
				string addressnum;
				string book;

				commMsg = commMsg.substr(2);

				output.clear();
				output.seekg(output.tellg(), ios::beg); //moves pointer back to beginning of file
				output.seekg(0, ios::beg);
				output.clear();
				getline(output, book);
				int pos = output.tellg();



				while (output.good())
				{
					for (int i = 0; i < book.size(); ++i) {
						if (book.at(i) == ' ') {
							addressnum = book.substr(0, i);
							break;
						}
					}
					//setting first name
					for (int i = addressnum.size() + 1; i < book.size(); ++i) {
						if (book.at(i) == ' ') {
							firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
							break;
						}
					} //setting last name
					for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
						if (book.at(i) == ' ') {
							lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
							break;
						}
					} //setting phonenumber
					book.append(" ");
					for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
					{
						if (book.at(i) == ' ') {
							phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
							break;
						}
					}

					if (phonenumber == commMsg)
						cout << "Found match" << endl;
						cout << book << endl;



					getline(output, book);


				}
			}
			else {

			cout << "404 Your search did not match any records" << endl;

				}
				output.close();
				}
			else
			{
				//error
				cout << "403 Message Format Error, try again." << endl;
			}



			//echoes message back to client
		logoutvalid:
			send(new_s, buf, bytesRec + 1, 0);

		}



		//close socket
		close:
		closesocket(new_s);
		//shutdown winsock
		WSACleanup();
	}

}

int client2(int count) {
	int checkloginuser = 0;
	bool loggedin = false;
	while (true)
	{


		//initialize winsock
		WSADATA winsData;
		WORD verReq = MAKEWORD(2, 2); //to request which version we want, 2.2

		int wsOk = WSAStartup(verReq, &winsData);
		if (wsOk != 0)
		{
			cerr << "Error initializing winsock. Try again. \n";
			return 0;
		}

		//create socket
		SOCKET listening = socket(PF_INET, SOCK_STREAM, 0); //takes address family, opens tcp socket, listening is our socket
		if (listening == INVALID_SOCKET)
		{
			cerr << "Socket creation failed. Try again. \n";
			return 0;
		}
		//                     more error handling?

		//bind socket to an ip address and port
		sockaddr_in sin; //fill in a hint structure//indicates preferred socket/protocol 
		sin.sin_family = PF_INET; //using version 4
		sin.sin_port = htons(SERVER_PORT2); //htons=host to network short
		sin.sin_addr.S_un.S_addr = INADDR_ANY; //to bind to any address

		bind(listening, (sockaddr*)&sin, sizeof(sin)); //binds to networking code, file descriptor

		//tell winsock , the socket is listening
		listen(listening, SOMAXCONN);
		//wait for a connection

		sockaddr_in client; //input address for client
		int clientSize = sizeof(client);

		SOCKET new_s = accept(listening, (sockaddr*)&client, &clientSize);
		if (new_s == INVALID_SOCKET)
		{
			cerr << "Invalid socket. Try again. \n";
			return 0;

		}


		char host[NI_MAXHOST]; //clients remote name
		char service[NI_MAXHOST]; // port the client connects to, service buffer

		memset(host, 0, NI_MAXHOST);  //setting to zero
		memset(service, 0, NI_MAXHOST);

		// check if we can get name info, if not, rely on what we have
		if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
		{
			cerr << host << "Connected on port" << service << endl;
		}
		else
		{
			inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			cerr << host << " connected on port" << ntohs(client.sin_port) << endl;
		}
		//close listening socket
		closesocket(listening);

		//accept message from client

		char buf[4096]; //mem for buf message
		send(new_s, to_string(count).c_str(), 4096, 0);

		while (true)
		{
			memset(buf, 0, 4096);

			//wait for client to send data
			int bytesRec = recv(new_s, buf, 4096, 0);
			if (bytesRec == SOCKET_ERROR)
			{
				cerr << "Error recieving. Try again. \n";
				break;
			}
			if (bytesRec == 0)
			{
				cerr << "Disconnected from client. \n"; //when client disconnects, or quits
				break;
			}

			// address book editing

			//command word being sent (add,delete,list,shutdown,quit)
			string commWord;
			//command message being sent
			string commMsg = string(buf, 0, bytesRec);
			//cerr << commMsg << endl;
			for (int i = 0; i < commMsg.size() - 1; i++)
			{
				if (commMsg.at(i) == ' ')
				{
					commWord = commMsg.substr(0, i); //command
					commMsg = commMsg.substr(i + 1); //second part of message after command
					break;
				}

			}

			if (loggedin)
			{

				if (commWord == "ADD")
				{
					output.open("Book.txt", ios::out | ios::in);
					//puts output to end of file
					output.seekg(output.tellg(), ios::end);
					//output count 
					output << ++addtracker << " " << left << setw(25) << commMsg << "\n";
					output.close();
					goto logoutvalid;
				}
				else if (commMsg == "SHUTDOWN") //comword orig
				{
					//checkloginuser
					if (loggedincheck[0] == true) {
						closesocket(new_s);
						WSACleanup();
						exit(0);
						break;
					}
					else
					{
						cout << "402 User not allowed to execute this command";
						output.seekg(output.tellg(), ios::beg);
					}
				}
				else if (commWord == "DELETE")
				{
					output.open("Book.txt", ios::out | ios::in);
					string book;
					string blank = "                             "; //replaces deleted message
					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					output >> book;
					int pos = output.tellg();

					while (output.good())
					{

						if (commMsg == book)
						{

							output.seekg(output.tellg(), ios::beg); //resets to where we currently are
							output.seekg(pos);


							output << blank << endl; //deletes
							break;
						}

						output >> book;
						pos = output.tellg();

						for (int i = 0; i < book.size() - 1; ++i)
						{
							if (isalnum(book.at(i))) // if character is true
							{
								book = book.substr(i);
								break;
							}
						}


					}

					if (!output.good())
					{
						cout << "Invalid address, try again." << endl;
						output.clear(); //sets bit to good
						output.seekg(output.tellg(), ios::beg);

					}

					// search for address
					output.seekg(32);
					// go back an int bit
					output.close();
					goto logoutvalid;
				}
				else if (commMsg == "LOGOUT")
				{

					loggedincheck[checkloginuser] = false;
					loggedin = false;
					cout << "200 OK" << endl;
					goto logoutvalid;

				}

			}

			if (commMsg == "LIST")
			{
				output.open("Book.txt", ios::out | ios::in);
				cout << "200 OK" << endl;
				string book;
				output.clear();
				//moves pointer back to beginning of file
				output.seekg(output.tellg(), ios::beg);
				output.seekg(0, ios::beg);
				output.clear();
				getline(output, book);

				//while good, keep printing
				while (output.good())
				{
					cout << book << endl;
					getline(output, book);
				}
				cout << book << endl;
				//clears error place in output
				output.clear();
				output.close();

			}
			else if (commWord == "LOGIN") {
				if (!loggedin) {
					login.open("login.txt", ios::out | ios::in);
					string userlogin;
					login.clear();
					//moves pointer back to beginning of file
					login.seekg(output.tellg(), ios::beg);
					login.seekg(0, ios::beg);
					login.clear();
					getline(login, userlogin);
					int pos = login.tellg();
					int usernum = 0;
					while (login.good()) {

						// if login doesnt match
						if (commMsg.size() == userlogin.size())
						{

							for (int i = 0; i < userlogin.size() - 1; ++i)
							{
								if (commMsg.at(i) != userlogin.at(i)) {
									goto skip;
								}

							}
							loggedincheck[usernum] = true;
							checkloginuser = usernum;
							loggedin = true;
							break;
						}
					skip:
						usernum++;
						// grabs entire line, causes good() to fail
						getline(login, userlogin);



					}
					if (loggedincheck[checkloginuser])
					{
						cout << "200 OK" << endl;
					}
					else
					{
						cout << "410 Wrong UserID and Password" << endl;
					}
					login.close();
				}

			}
			else if (commMsg == "WHO") {
				cout << "200 OK" << endl << "The list of active users:" << endl;
				if (loggedincheck[0])
					cout << "root" << "  127.0.0.1 \n";
				if (loggedincheck[1])
					cout << "john" << "  127.0.0.1 \n";
				if (loggedincheck[2])
					cout << "david" << "  127.0.0.1 \n";
				if (loggedincheck[3])
					cout << "mary" << "  127.0.0.1 \n";

			}
			else if (commMsg == "QUIT")
			{
				loggedincheck[checkloginuser] = false;
				loggedin = false;
				cout << "200 OK" << endl;
				cout << "Client now closed." << endl;
				goto close;
				//closes just client

			}
			else if (commWord == "LOOKUP")
			{
				output.open("Book.txt", ios::out | ios::in);
				if (commMsg.at(0) == '1')
				{

					string firstname;
					string lastname;
					string phonenumber;
					string addressnum;
					string book;
					commMsg = commMsg.substr(2);

					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					getline(output, book);
					int pos = output.tellg();



					while (output.good())
					{
						for (int i = 0; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								addressnum = book.substr(0, i);
								break;
							}
						}//setting first name
						for (int i = addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
								break;
							}
						} //setting last name
						for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
								break;
							}
						} //setting phone number
						for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
						{
							if (book.at(i) == ' ') {
								phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
								break;
							}
						}

						if (firstname == commMsg) 
							cout << "Found match" << endl;
							cout << book << endl;
						



						getline(output, book);


					}
				}
				else if (commMsg.at(0) == '2')
				{

					string firstname;
					string lastname;
					string phonenumber;
					string addressnum;
					string book;
					commMsg = commMsg.substr(2);

					output.clear();
					//moves pointer back to beginning of file
					output.seekg(output.tellg(), ios::beg);
					output.seekg(0, ios::beg);
					output.clear();
					getline(output, book);
					int pos = output.tellg();



					while (output.good())
					{
						for (int i = 0; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								addressnum = book.substr(0, i);
								break;
							}
						}//setting first name
						for (int i = addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
								break;
							}
						} //setting last name
						for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
								break;
							}
						} //setting phone number
						for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
						{
							if (book.at(i) == ' ') {
								phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
								break;
							}
						}

						if (lastname == commMsg)
							cout << "Found match" << endl;
							cout << book << endl;


						getline(output, book);


					}
				}
				else if (commMsg.at(0) == '3') {

					string firstname;
					string lastname;
					string phonenumber;
					string addressnum;
					string book;

					commMsg = commMsg.substr(2);

					output.clear();
					output.seekg(output.tellg(), ios::beg); //moves pointer back to beginning of file
					output.seekg(0, ios::beg);
					output.clear();
					getline(output, book);
					int pos = output.tellg();



					while (output.good())
					{
						for (int i = 0; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								addressnum = book.substr(0, i);
								break;
							}
						}
						//setting first name
						for (int i = addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								firstname = book.substr(addressnum.size() + 1, i - (addressnum.size() + 1));
								break;
							}
						} //setting last name
						for (int i = firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i) {
							if (book.at(i) == ' ') {
								lastname = book.substr(firstname.size() + 1 + addressnum.size() + 1, i - (firstname.size() + 1 + addressnum.size() + 1));
								break;
							}
						} //setting phonenumber
						book.append(" ");
						for (int i = lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1; i < book.size(); ++i)
						{
							if (book.at(i) == ' ') {
								phonenumber = book.substr((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1), i - ((lastname.size() + 1 + firstname.size() + 1 + addressnum.size() + 1)));
								break;
							}
						}

						if (phonenumber == commMsg)
							cout << "Found match" << endl;
							cout << book << endl;



						getline(output, book);


					}
				}
				else {

					cout << "404 Your search did not match any records" << endl;

				}
				output.close();
			}
			else
			{
				//error
				cout << "403 Message Format Error, try again." << endl;
			}



			//echoes message back to client
		logoutvalid:
			send(new_s, buf, bytesRec + 1, 0);

		}



		//close socket
	close:
		closesocket(new_s);
		//shutdown winsock
		WSACleanup();
	}

}
#include <iostream>
#include <string>
#include <fstream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main()
{
	//Initialize winsock
	WSADATA wsData;
	WORD version = MAKEWORD(2, 2);

	int wsOk = WSAStartup(version, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return;
	}
	
	//Create socket
	//A socket is an endpoint, it's a number, in unix it's called a file descriptor
	//"This is the reference number we're going to use to access the specified port at the specified network address
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create socket! Quitting" << endl;
		return;
	}

	//Bind ip / port to socket
	//Networking is big endian
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	
	//Tell winsock to listen on this socket
	//Just labels the socket as one that listens
	listen(listening, SOMAXCONN);
	
	//Wait for connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET)
	{
		cerr << "Can't accept client socket! Quitting" << endl;
		return;
	}

	char host[NI_MAXHOST]; //Client's remote name (only works sometimes)
	char service[NI_MAXSERV]; //Service / port that the client is connected to
	
	ZeroMemory(host, NI_MAXHOST); //Same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	//Check to see if we can get the client name
	//If not, we just display the client's ip
	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	//Close listening socket
	closesocket(listening);

	//Accepts and echos messages back to client
	const int bufLen = 4096;
	char buf[bufLen]; //Change this later, so we can transmit more than 4096
	string outFileLoc = "C:/test.txt";
	
	while (true)
	{
		ZeroMemory(buf, bufLen);

		//Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, bufLen, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "Error receiving data! Quitting" << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			cerr << "Client disconnected? Quitting" << endl;
			break;
		}

		//Write bytes to string for easier interpretation
		string bytesReceivedAsString(buf, bytesReceived);

		//Write text to a file
		//ofstream outFile;
		//outFile.open(outFileLoc);
		//outFile << bytesReceivedAsString << endl;
		//outFile.close();

		cout << "Wrote file to " << bytesReceivedAsString << endl;

		//Echo message back to client
		//send tells you the number of bytes it sent
		//We have to add 1 to bytesReceived to include the trailing 0 that we ignore when we accept the message originally
		send(clientSocket, buf, bytesReceived + 1, 0);
	}
	
	//Close socket
	closesocket(clientSocket);

	//Clean up winsock
	WSACleanup();
}
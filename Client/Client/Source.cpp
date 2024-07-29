#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;


SOCKET Connection;


enum PacketType : int
{
	type_MESSAGE = 1,
	type_NICKNAME = 2
};

int UsernameHandler(string&);
void ClientHandler();


int UsernameHandler(string& username)
{
	cout << "Enter your nickname (up to 24 characters): " << endl;
	cin >> username;
	int usrnme_size = username.size();
	if (usrnme_size > 24 || usrnme_size == 0)
	{
		cout << "Incorrect username length." << endl;
		UsernameHandler(username);
	}
	else
	{
		return usrnme_size;
	}
}

void ClientHandler()
{
	int msg_size;

	while (true)
	{

		/*��������� ������� ��������� � �������*/
		if (recv(Connection, (char*)&msg_size, sizeof(msg_size), NULL) == SOCKET_ERROR)
		{
			cout << "Server didn't respond. Closing the connection." << endl;
			exit(1);
		}
		
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		/*��������� ��������� � �������*/
		recv(Connection, msg, msg_size, NULL);
		cout << msg << endl;
		
		delete[] msg;
	}
}

int main(int argc, char* argv[]) 
{
	/*��������� � �������*/
	system("chcp 1251");

	PacketType packettype;
	string username;

	/*������������� ���������� winsock*/
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); /*������ winsock*/
	/*�������� ���������� winsock*/
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		cout << "Error loading winsock library" << endl;
		exit(1);
	}

	/*����� ������*/
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	/*����������� � ��������� ������ �������, � ������ ������ -
	- local host*/
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*����������� � ��������� ����� �������*/
	addr.sin_port = htons(1111);
	/*��������� �������� ����������*/
	addr.sin_family = AF_INET;

	/*����� ��� ����������� � �������*/
	Connection = socket(AF_INET, SOCK_STREAM, NULL);

	/*������� ����������� � ������� �� ������ � �����*/
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) 
	{
		cout << "Error connecting to server." << endl;
		exit(1);
	}
	else
	{
		cout << "Connected to server " << endl;

		packettype = type_NICKNAME;
		int usrnme_size = UsernameHandler(username);

		if (send(Connection, (char*)&packettype, sizeof(PacketType), NULL) == SOCKET_ERROR)
		{
			cout << "Server didn't respond. Closing the connection." << endl;
			exit(1);
		}
		send(Connection, (char*)&usrnme_size, sizeof(int), NULL);
		send(Connection, username.c_str(), usrnme_size, NULL);
	}

	/*�������� ������ ������ ��� ��������� ��������� � �������
	��������� �� ������� ClientHandler;
	����� ����� ������� ����� ����� ����������.*/
	if (CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)NULL, NULL, NULL) == NULL)
	{
		cout << "Error starting new thread" << endl;
		exit(1);
	}

	/*���� ���������*/
	string msg1;
	while (true)
	{
		packettype = type_MESSAGE;

		getline(cin, msg1);
		if (msg1.size() == 0)
		{
			continue;
		}
		msg1 = (username + ": " + msg1);
		int msg1_size = msg1.size();

		if (send(Connection, (char*)&packettype, sizeof(PacketType), NULL) == SOCKET_ERROR)
		{
			cout << "Server didn't respond. Closing the connection." << endl;
			exit(1);
		}
		/*�������� ������� ��������� �� ������*/
		send(Connection, (char*)&msg1_size, sizeof(int), NULL);
		/*�������� ��������� �� ������*/
		send(Connection, msg1.c_str(), msg1_size, NULL);

		/*�������� ����� ���������*/
		Sleep(10);
	}

	system("pause");
	return 0;
}
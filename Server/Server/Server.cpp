#include <iostream>
#include <string>
#include <winsock2.h>
#include <map>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;


const int MAX_CONNECTIONS = 100;
/*массив сокетов - подключенных клиентов*/
SOCKET Connections[MAX_CONNECTIONS];
/*счётчик подключений*/
int Counter = 0;
map <string, int> ConName_Map;


enum PacketType : int
{
	type_MESSAGE = 1,
	type_NICKNAME = 2
};

bool ConExists(string&);
void InRequestsHandler(SOCKET, int);
void ClientHandler(int);


/*проверка на существование соединения по ключу*/
bool ConExists(string& searchName)
{
	auto it = ConName_Map.find(searchName);
	if (it == ConName_Map.end())
	{
		return false;
	}

	return true;
}

/*обработка запросов на подключение*/
void InRequestsHandler(SOCKET tempConnection, int index)
{
	PacketType packettype;
	int usrnme_size;

	if (recv(tempConnection, (char*)&packettype, sizeof(PacketType), NULL) == SOCKET_ERROR)
	{
		closesocket(Connections[index]);
		cout << "Client #" << index << " disconnected.\n" <<
			"SOCKET #" << index << " closed." << endl;
		return;
	}

	recv(tempConnection, (char*)&usrnme_size, sizeof(int), NULL);

	char* usrnme = new char[usrnme_size + 1];
	usrnme[usrnme_size] = '\0';

	recv(tempConnection, usrnme, usrnme_size, NULL);

	string username(usrnme);

	delete[] usrnme;
	usrnme = nullptr;

	switch (ConExists(username))
	{

	case true:
	{
		InRequestsHandler(tempConnection, index);
	}

	case false:
	{
		ConName_Map.insert(make_pair(username, index));
		cout << "Created new connection with " << username
			<< " name and " << index << " index." << endl;

		cout << "\tClient #" << index << " connected successfully." << endl;

		/*добавляем новое соединение в массив сокетов*/
		Connections[index] = tempConnection;
		/*инкрементируем счётчик соединений*/
		Counter++;

		thread th2(ClientHandler, index);
		th2.detach();

		break;
	}

	}
	

}

void ClientHandler(int index)
{
	/*тип пакета*/
	PacketType packettype;
	/*размер сообщения*/
	int msg_size;
	
	while (true)
	{
		/*получение отправленного с клиента типа пакета*/
		if (recv(Connections[index], (char*)&packettype, sizeof(PacketType), NULL) == SOCKET_ERROR)
		{
			closesocket(Connections[index]);
			cout << "Client #" << index << " disconnected.\n" <<
				"SOCKET #" << index << " closed." << endl;
			return;
		}

		switch (packettype)
		{

		case type_MESSAGE:
		{
			/*получение отправленного с клиента размера сообщения*/
			recv(Connections[index], (char*)&msg_size, sizeof(msg_size), NULL);

			char* msg = new char[msg_size + 1];
			msg[msg_size] = '\0';
			/*получение отправленного с клиента сообщения*/
			recv(Connections[index], msg, msg_size, NULL);
			for (int i = 0; i < Counter; i++)
			{
				/*если индекс клиента, отправившего сообщения совпадает с текущим
				индексом сокета из массива подключений, то пропускаем итерацию*/
				if (i == index)
				{
					continue;
				}
				else
				{
					/*отправляем размер принятого от клиента сообщения в оставшиеся клиенты*/
					send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
					/*отправляем принятое от клиента сообщения в оставшиеся клиенты*/
					send(Connections[i], msg, msg_size, NULL);
				}
			}

			delete[] msg;
			break;
		}

		case type_NICKNAME:
		{
			break;
		}

		default:
		{
			break;
		}

		}
	}
}

int main(int argc, char* argv[])
{
	/*кириллица в консоли*/
	system("chcp 1251");

	/*инициализация библиотеки winsock*/
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1); /*версия winsock*/
	/*загрузка библиотеки winsock*/
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		cout << "Error loading winsock library" << endl;
		exit(1);
	}

	/*адрес сокета*/
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	/*local host*/
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*занимается порт 1111*/
	addr.sin_port = htons(1111);
	/*семейство интернет протоколов*/
	addr.sin_family = AF_INET;

	/*создаётся сокет*/
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	/*привязка к сокету адреса сокета*/
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	/*прослушивание порта и адреса занимаемыми сокетом,
	с заданным максимальным количеством подключений*/
	listen(sListen, SOMAXCONN);

	/*новый сокет для текущего соединения с клиентом*/
	SOCKET newConnection;


	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{

		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);


		/*если клиент не подключился к серверу*/
		if (newConnection == 0)
		{
			cout << "Error connecting client to server" << endl;
			exit(1);
		}
		else
		{
			cout << "Client #" << i << " sent connection requst." << endl;
			thread th(InRequestsHandler, newConnection, i);
			th.detach();
		}

	}

	system("pause");
	return 0;

}
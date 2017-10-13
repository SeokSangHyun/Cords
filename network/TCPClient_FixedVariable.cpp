#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE    40

using namespace std;


// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

//	���� ������ ������
void sendToS(SOCKET& s) {

	int temp_retval;
	int len;
	char buf[BUFSIZE];
	char fileName[50];
	int fileSize = 0, sizeCount=0;
	FILE *f;

	

	//
	//file open�� �Ѵ�.
	//
	//
	//	�������̸� ���� �޾ƿ´�.
	//	send(s, (char*)len, sizeof(int), 0);
	//�������̸� �޾ƿ´�.
	//	send(s, buf, len, 0);


	cout << "�ҷ��� ���ϸ��� �Է����ּ���(Ȯ���� �����. ex).txt, .jpg, .png)";
	cin >> fileName;
	f = fopen(fileName, "rb");
	if (f == NULL) err_quit("FileName error");

	//���� �̸� ���� / ���� �̸� / ���� ��ü ����
	len = strlen(fileName);
	temp_retval = send(s, (char *)&len, sizeof(int), 0);

	temp_retval = send(s, fileName, len, 0);

	fseek(f, 0, SEEK_END);
	fileSize = ftell(f);
	temp_retval = send(s, (char*)&fileSize, sizeof(int), 0);
	fseek(f, 0, SEEK_SET);

	len = 0;
	ZeroMemory(buf, BUFSIZE);
	while (fileSize != EOF) {
		// (��ŭ ����Ʈ�� �о� �����´���)  <temp_retval = send(����, ����������, ����, �÷���(0));>

		fgets(buf, BUFSIZE, f);
		len = strlen(buf);
		temp_retval = send(s, (char*)&len, sizeof(int), 0);
		if (temp_retval == SOCKET_ERROR)
		{
			err_display("send()"); break;
		}

		temp_retval = send(s, buf, len, 0);
		if (temp_retval == SOCKET_ERROR)
		{
			err_display("send()"); break;
		}

		sizeCount += len;
	}
	ZeroMemory(buf, BUFSIZE);
	temp_retval = send(s, buf, len, 0);

	printf("[TCP Ŭ���̾�Ʈ] %d+%d����Ʈ�� "
		"���½��ϴ�.\n", sizeof(int), temp_retval);
}


int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");


	sendToS(sock);

	// closesocket()
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
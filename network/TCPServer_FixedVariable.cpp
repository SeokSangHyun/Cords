#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

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

// ����� ���� ������ ���� �Լ�
int recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
  	char *ptr = buf;
	int left = len;

	while(left > 0){
		received = recv(s, buf, left, flags);
		if(received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if(received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}


//clint������ �ޱ�
void recvToC(SOCKET& s, char *buf, int len, int flags, SOCKADDR_IN csa)
{
	int temp_retvl;
	int fileSize, sizeCount = 0;
	char temp[50];

	FILE *f;
	/*
	�����̸��� �޾Ƽ� Ȯ���Ѵ�.
		>�ִٸ� . �߰��� �Է�
		>���ٸ� . ���� �����Ѵ�.

	�޾ƿ� �ڷḦ "�ϴ�" ��� �Ѵ�.
	*/

	ZeroMemory(buf, BUFSIZE);
	// ���ϸ� ���� / ���� �̸� / ���� ũ��
	temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
	temp_retvl = recvn(s, buf, len, flags);

	temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
	fileSize = len;


	//���� ����
	f = fopen(buf, "wb");
	if (f == NULL) err_quit("file havn't");

	//client���� �����͸� �޾Ƽ� ���Ͽ� ����
	while (sizeCount != fileSize) {
	ZeroMemory(buf, BUFSIZE);

		// ������ �ޱ�(���� ����)
		temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
		if (temp_retvl == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (temp_retvl == 0) break;

		// ������ �ޱ�(���� ����)
		temp_retvl = recvn(s, buf, len, flags);
		if (temp_retvl == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (temp_retvl == 0) break;


		fputs(buf, f);
		sizeCount += len;
	}


	// ���� ������ ���
	buf[temp_retvl] = '\0';
	printf("[TCP/%s:%d] %s\n", inet_ntoa(csa.sin_addr), ntohs(csa.sin_port), buf);

	fclose(f);
}



int main(int argc, char *argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if(retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if(retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	int len = 0;
	char buf[BUFSIZE];

	while(1){
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if(client_sock == INVALID_SOCKET){
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		recvToC(client_sock, buf, len, 0, clientaddr);

		// closesocket()
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}
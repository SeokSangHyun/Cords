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


// 소켓 함수 오류 출력 후 종료
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

// 소켓 함수 오류 출력
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

//	소켓 데이터 보내기
void sendToS(SOCKET& s) {

	int temp_retval;
	int len;
	char buf[BUFSIZE];
	char fileName[50];
	int fileSize = 0, sizeCount=0;
	FILE *f;

	

	//
	//file open을 한다.
	//
	//
	//	고정길이를 먼저 받아온다.
	//	send(s, (char*)len, sizeof(int), 0);
	//가변길이를 받아온다.
	//	send(s, buf, len, 0);


	cout << "불러올 파일명을 입력해주세요(확장자 명까지. ex).txt, .jpg, .png)";
	cin >> fileName;
	f = fopen(fileName, "rb");
	if (f == NULL) err_quit("FileName error");

	//파일 이름 길이 / 파일 이름 / 파일 전체 길이
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
		// (얼만큼 바이트를 읽어 보내냈는지)  <temp_retval = send(소켓, 보낼데이터, 길이, 플래그(0));>

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

	printf("[TCP 클라이언트] %d+%d바이트를 "
		"보냈습니다.\n", sizeof(int), temp_retval);
}


int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
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

	// 윈속 종료
	WSACleanup();
	return 0;
}
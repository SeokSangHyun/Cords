#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512

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

// 사용자 정의 데이터 수신 함수
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


//clint데이터 받기
void recvToC(SOCKET& s, char *buf, int len, int flags, SOCKADDR_IN csa)
{
	int temp_retvl;
	int fileSize, sizeCount = 0;
	char temp[50];

	FILE *f;
	/*
	파일이름을 받아서 확인한다.
		>있다면 . 추가로 입력
		>없다면 . 새로 생성한다.

	받아온 자료를 "일단" 출력 한다.
	*/

	ZeroMemory(buf, BUFSIZE);
	// 파일명 길이 / 파일 이름 / 파일 크기
	temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
	temp_retvl = recvn(s, buf, len, flags);

	temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
	fileSize = len;


	//파일 열기
	f = fopen(buf, "wb");
	if (f == NULL) err_quit("file havn't");

	//client에서 데이터를 받아서 파일에 저장
	while (sizeCount != fileSize) {
	ZeroMemory(buf, BUFSIZE);

		// 데이터 받기(고정 길이)
		temp_retvl = recvn(s, (char *)&len, sizeof(int), flags);
		if (temp_retvl == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (temp_retvl == 0) break;

		// 데이터 받기(가변 길이)
		temp_retvl = recvn(s, buf, len, flags);
		if (temp_retvl == SOCKET_ERROR) {
			err_display("recv()"); break;
		}
		else if (temp_retvl == 0) break;


		fputs(buf, f);
		sizeCount += len;
	}


	// 받은 데이터 출력
	buf[temp_retvl] = '\0';
	printf("[TCP/%s:%d] %s\n", inet_ntoa(csa.sin_addr), ntohs(csa.sin_port), buf);

	fclose(f);
}



int main(int argc, char *argv[])
{
	int retval;

	// 윈속 초기화
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

	// 데이터 통신에 사용할 변수
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

		// 접속한 클라이언트 정보 출력
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		recvToC(client_sock, buf, len, 0, clientaddr);

		// closesocket()
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
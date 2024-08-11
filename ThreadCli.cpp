#include <stdio.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")
#define NAME_SIZE 255
#define MAX_BUF 1024

char szName[NAME_SIZE]{ "def" };
char szMsg[MAX_BUF];

unsigned WINAPI handleSend(void* arg) {
	SOCKET sockCli = *(SOCKET*)arg;
	int iLen = 0;
	char szNkMsg[MAX_BUF + NAME_SIZE]{ 0 };
	while (1) {
		memset(szMsg, 0, MAX_BUF);
		fgets(szMsg, MAX_BUF, stdin);
		if (!strcmp(szMsg, "q\n") || !strcmp(szMsg, "Q\n")) {
			closesocket(sockCli);
			exit(0);
		}
		sprintf_s(szNkMsg, "%s: %s", szName, szMsg);
		send(sockCli, szNkMsg, strlen(szNkMsg), 0);
		//send(sockCli, szNkMsg, sizeof(szNkMsg), 0);
	}
	return 0;
}
unsigned WINAPI handleRecv(void* arg) {
	SOCKET sockCli = *(SOCKET*)arg;
	int iLen = 0;
	char szNkMsg[MAX_BUF + NAME_SIZE]{ 0 };
	while (1) {
		memset(szMsg, 0, MAX_BUF);
		iLen = recv(sockCli, szNkMsg, sizeof(szNkMsg), 0);
		if (iLen == -1) {
			return 2;
		}
		szNkMsg[iLen] = 0;
		fputs(szNkMsg, stdout);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Input Nickname\n");
		system("pause");
		return -1;
	}
	sprintf_s(szName, "[%s]", argv[1]);
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	SOCKET sockCli = socket(AF_INET, SOCK_STREAM, 0);
	if (sockCli == INVALID_SOCKET) {
		printf("socket errorno:%d\n", WSAGetLastError());
		return -1;
	}

	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.S_un.S_addr = inet_addr("192.168.31.45");
	addrSrv.sin_port = htons(54321);

	if (connect(sockCli, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("connect errorno:%d\n", WSAGetLastError());
		system("pause");
		return -1;
	}

	HANDLE hSend= (HANDLE)_beginthreadex(NULL, 0, handleSend, (void*)&sockCli, 0, NULL);
	HANDLE hRecv= (HANDLE)_beginthreadex(NULL, 0, handleRecv, (void*)&sockCli, 0, NULL);

	WaitForSingleObject(hSend,INFINITE);
	WaitForSingleObject(hRecv,INFINITE);

	closesocket(sockCli);
	WSACleanup();

	return 0;
}
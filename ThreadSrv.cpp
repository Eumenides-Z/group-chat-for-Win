#include <stdio.h>
#include <windows.h>
#include <process.h>
#pragma comment(lib,"ws2_32.lib")

#define MAX_SUB 100
#define MAX_BUF 1024

SOCKET sckCli[MAX_SUB];
int cliCnt{ 0 };
HANDLE hMutex;

void sendGrp(char* msg, int iLen) {
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cliCnt; i++) {
		send(sckCli[i], msg, iLen, 0);
	}
	ReleaseMutex(hMutex);
}

unsigned WINAPI handleCli(void* arg) {
	SOCKET sckConn = *(SOCKET*)arg;
	int iLen{ 0 };
	char msgbuf[MAX_BUF]{ 0 };
	while (1) {
		iLen = recv(sckConn, msgbuf, sizeof(msgbuf), 0);
		if (iLen != -1) {
			sendGrp(msgbuf, iLen);
		}
		else {
			break;
		}
	}
	printf("connected,now number %d\n", cliCnt);
	WaitForSingleObject(hMutex, INFINITE);
	for (int i = 0; i < cliCnt; i++) {
		if (sckCli[i] == sckConn) {
			while (i < cliCnt) {
				sckCli[i] = sckCli[i + 1];
				i++;
			}
			break;
		}
	}
	cliCnt--;
	printf("disconnect,now %d remain\n", cliCnt);
	ReleaseMutex(hMutex);
	closesocket(sckConn);
	return 0;
}


int main() {
	printf("Thread Chat Server\n");
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	hMutex = CreateMutex(NULL, FALSE, NULL);

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET) {
		printf("socket errorno:%d\n", WSAGetLastError());
		return -1;
	}

	SOCKADDR_IN mySrv;
	mySrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	mySrv.sin_family = AF_INET;
	mySrv.sin_port = htons(54321);

	if (SOCKET_ERROR == bind(sockSrv, (SOCKADDR*)&mySrv, sizeof(SOCKADDR))) {
		printf("bind errorno:%d\n", WSAGetLastError());
		return -1;
	}

	if (SOCKET_ERROR == listen(sockSrv, 5)) {
		printf("listen errorno:%d\n", WSAGetLastError());
		return -1;
	}
	printf("start chat\n");

	SOCKADDR_IN addrCli;
	int len = sizeof(SOCKADDR);
	while (1) {
		SOCKET sckConn = accept(sockSrv, (SOCKADDR*)&addrCli, &len);
		WaitForSingleObject(hMutex, INFINITE);
		sckCli[cliCnt] = sckConn;
		cliCnt++;
		ReleaseMutex(hMutex);
		HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, handleCli, (void*)&sckConn, 0, NULL);
		printf("connect to client %s, num = %d\n", inet_ntoa(addrCli.sin_addr), cliCnt);

	}
	closesocket(sockSrv);
	WSACleanup();


	return 0;
}
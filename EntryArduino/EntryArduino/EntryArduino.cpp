// EntryArduino.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EntryArduino.h"
#include "Serial.h"
#include <atlstr.h>
#include "sckt.h"
#include <regex>
#include <string.h>
#include "sha1.h"

using namespace cryptlite;

#define MAX_LOADSTRING 100
#define BUFFER_SIZE 10000
#define DEFAULT_PORT "23518"
#define SERIAL_INTERVAL 15
#define WM_SOCKET		104
#define WM_SERIAL		(WM_USER + 0x0001)
#define SERVER_HASH_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
char szHistory[10000];

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
char InputData[BUFFER_SIZE];

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
VOID UpdateValue(char * inputData, int dataLength);
VOID CALLBACK InitSocket(HWND hWnd);
VOID CALLBACK connectSerial(HWND hWnd);
BOOLEAN CALLBACK ReadSerial();

Serial* SP = NULL;
SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;
int readResult = 0;
int failCount = 0;
std::string serverHash;
BOOL isSocketConnected = FALSE;
BOOL isSocketEstablished = FALSE;

char remainSerialValue = 0;

int analogValue[6];
int digitalValue[14];

char socketInput[1024];
int payloadLength;
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ENTRYARDUINO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}



	
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENTRYARDUINO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}



	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENTRYARDUINO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ENTRYARDUINO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	DWORD       dwStyle;
	HDC hdc;
	PAINTSTRUCT ps;
	dwStyle = (WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU);
	
	hInst = hInstance; // Store instance handle in our global variable
	
	hWnd = CreateWindow(
	szWindowClass,
	_T("Entry Arduino"),
	dwStyle,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	300,
	150,
	NULL,
	NULL,
	hInstance,
	NULL
	);
	
	if (!hWnd)
		return FALSE;
		
	ShowWindow(hWnd, nCmdShow);
	hdc = BeginPaint(hWnd, &ps);

	RECT rect;
	GetClientRect(hWnd, &rect);
	InvalidateRect(hWnd, &rect, TRUE);

	PostMessage(hWnd, WM_SERIAL, NULL, FD_CLOSE);
	InitSocket(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:


		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			SP->disconnect();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		/*
		if (isSocketConnected)
			TextOut(hdc,
				40, 50,
				L"Entry connected", _tcslen(L"Entry connected"));
		else
			TextOut(hdc,
			40, 50,
			L"Entry disconnected", _tcslen(L"Entry disconnected"));
			*/
		if (SP && SP->IsConnected())
			TextOut(hdc,
				24, 40,
				L"아두이노가 연결되었습니다.", _tcslen(L"아두이노가 연결되었습니다."));
		else
			TextOut(hdc,
			24, 40,
			L"아두이노가 연결되지 않았습니다.", _tcslen(L"아두이노가 연결되지 않았습니다."));
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		// shutdown the connection since we're done
		int iResult;

		// socket cleanup
		closesocket(ClientSocket);
		SP->disconnect();
		WSACleanup();
		PostQuitMessage(0);
		return 0;

	case WM_SOCKET:
	{
		switch (WSAGETSELECTEVENT(lParam))
		{
			case FD_READ:
			{
				char szIncoming[1024];
				ZeroMemory(szIncoming, sizeof(szIncoming));
				int inDataLength;
				inDataLength = recv(ClientSocket,
					(char*)szIncoming,
					sizeof(szIncoming) / sizeof(szIncoming[0]),
					0);
				
				

				if (strncmp(szIncoming, "GET", 3) == 0)
				{
					char * pch;
					char * context;
					pch = strtok_s(szIncoming, "\n", &context);
					char hashHeader[20] = "Sec-WebSocket-Key: ";
					while (pch != NULL)
					{
						if (strncmp(pch, hashHeader, 18) == 0) {
							std::string clientHash;
							clientHash = pch;
							clientHash = clientHash.substr(19, clientHash.length() - 20) + SERVER_HASH_KEY;
							clientHash = clientHash.c_str();
							serverHash = sha1::hash_base64(clientHash.data());
							break;
						}
						pch = strtok_s(NULL, "\n", &context);
					}
					if (serverHash.size()) {
						PostMessage(hWnd, WM_SOCKET, wParam, FD_WRITE);
					}
				}
				else if (isSocketEstablished)
				{
					int opcode = szIncoming[0] & 0x0F;
					bool isMask = szIncoming[1] >> 7;
					payloadLength = szIncoming[1] & 0x7F;
					char maskingKeys[4];
					ZeroMemory(socketInput, sizeof(socketInput));
					if (opcode == 8) {
						closesocket(ClientSocket);
						isSocketConnected = FALSE;

						isSocketEstablished = FALSE;
						InitSocket(hWnd);
						break;
					}
					if (payloadLength < 126)
					{
						if (isMask)
						{
							maskingKeys[0] = szIncoming[2];
							maskingKeys[1] = szIncoming[3];
							maskingKeys[2] = szIncoming[4];
							maskingKeys[3] = szIncoming[5];;
						}
						for (int i = 0; i < payloadLength; ++i) {
							socketInput[i] = szIncoming[i + 6] ^ maskingKeys[i % 4];
						}

					}
					SendMessage(hWnd, WM_SERIAL, wParam, FD_READ);
					SendMessage(hWnd, WM_SOCKET, wParam, FD_WRITE);
				}
			}

				break;


			case FD_WRITE:
			{
				if (serverHash.size()) {
					std::string header;
					header = "HTTP/1.1 101 Switching Protocols\r\n";
					header += "Upgrade: websocket\r\n";
					header += "Connection: Upgrade\r\n";
					header += "Sec-WebSocket-Accept : " + serverHash + "\r\n\r\n";

					serverHash = "";
					send(ClientSocket, header.c_str(), header.length(), 0);
					isSocketEstablished = TRUE;
				}
				else if (isSocketEstablished) {
					char socketHeader[10];
					ZeroMemory(socketHeader, sizeof(socketHeader));
					std::string output = "{\"d\":\"";
					for (int i = 0; i < 14; ++i)
					{
						if (digitalValue[i]>0)
							output += "1";
						else
							output += "0";
					}
					output += "\",";
					for (int i = 0; i < 6; ++i)
					{
						char str[30];
						sprintf_s(str, "\"a%d\":%d,", i, analogValue[i]);
						output += str;
					}
					output = output.substr(0, output.size()-1);
					output += "}";

					socketHeader[0] = 0x81;
					socketHeader[1] = 0x7F & output.size();
					output = socketHeader + output;
					send(ClientSocket, output.data(), output.size(), 0);
				}
				
			}
				break;

			case FD_CLOSE:
			{
				closesocket(ClientSocket);
				isSocketConnected = FALSE;

				isSocketEstablished = FALSE;
				InitSocket(hWnd);
			}
				break;

			case FD_ACCEPT:
			{
				int size = sizeof(sockaddr);
				ClientSocket = accept(ListenSocket, NULL, NULL);
				if (ClientSocket == INVALID_SOCKET)
				{
					int nret = WSAGetLastError();
					WSACleanup();
				}
				int iResult;
				iResult = WSAAsyncSelect(ClientSocket,
					hWnd,
					WM_SOCKET,
					(FD_READ | FD_WRITE | FD_CLOSE));
				if (iResult == SOCKET_ERROR) {
					closesocket(ClientSocket);
					WSACleanup();

				}

				closesocket(ListenSocket);

				isSocketConnected = TRUE;
			}
			break;
		}
		break;
	}
	case WM_SERIAL:
	{
		switch (WSAGETSELECTEVENT(lParam)) {
			case FD_READ:
			{
							
				if (SP->IsConnected())
				{
					BOOLEAN serialState;
					SP->WriteData(socketInput, payloadLength);
					serialState = ReadSerial();
					if (!serialState)
						failCount++;
					else
						failCount = 0;
					if (failCount > 10) {

						RECT rect;
						GetClientRect(hWnd, &rect);
						InvalidateRect(hWnd, &rect, TRUE);

						PostMessage(hWnd, WM_SERIAL, wParam, FD_CLOSE);

					}
				}
			}
				break;
			case FD_CLOSE:
			{
				if (SP)
					SP->disconnect();
				SendMessage(hWnd, WM_PAINT, wParam, NULL);
				connectSerial(hWnd);
				failCount = 0;
			}
				break;
			case FD_ACCEPT:
			{
							  RECT rect;
							  GetClientRect(hWnd, &rect);
							  InvalidateRect(hWnd, &rect, TRUE);
							  SendMessage(hWnd, WM_PAINT, wParam, NULL);
			}
				break;
			case FD_WRITE:
			{

			}
				break;
			}
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOLEAN CALLBACK ReadSerial()
{
	// Receive until the peer shuts down the connection
	int dataLength = BUFFER_SIZE;

	ZeroMemory(InputData, sizeof(InputData));

	if (SP->IsConnected()) {
		readResult = SP->ReadData(InputData, dataLength);
		UpdateValue(InputData, readResult);
		if (readResult < 0)
			return FALSE;
		else
			return TRUE;
	}
	return FALSE;

} 

VOID CALLBACK InitSocket(HWND hWnd)
{

	//socket
	WSADATA wsaData;
	int iResult;


	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[BUFFER_SIZE];
	int recvbuflen = BUFFER_SIZE;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);

	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();

	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();

	}

	iResult = WSAAsyncSelect(ListenSocket,
		hWnd,
		WM_SOCKET,
		(FD_CLOSE | FD_ACCEPT));
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		MessageBox(NULL, L"fail", NULL, NULL);

	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();

	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();

	}


	RECT rect;
	GetClientRect(hWnd, &rect);
	InvalidateRect(hWnd, &rect, TRUE);

}

// process serial data to value and store to memory
VOID UpdateValue(char * inputData, int dataLength)
{
	for (int i=0; i<dataLength; i++)
	{
		char data = inputData[i];
		
		if (data >> 7)
		{
			if (!((data >> 6) & 0x01)) {
				int port = (data >> 2) & 0x0f;
				digitalValue[port] = data & 0x01;
				remainSerialValue = 0;
			}
			else {
				remainSerialValue = data;
			}
		}
		else
		{
			if (remainSerialValue)
			{
				BOOL isAnalog = (remainSerialValue >> 6) & 0x01;
				if (isAnalog)
				{
					int port = (remainSerialValue >> 3) & 0x07;
					analogValue[port] = ((remainSerialValue & 0x07) << 7) +
						(data & 0x7F);
				}
				else
				{
				}
				remainSerialValue = 0;
			}
		}
	}
}

VOID CALLBACK connectSerial(HWND hWnd) {
	char portName[12];
	BOOL isConnected = FALSE;
	if (SP && SP->IsConnected())
		return;
	for (int i = 0; i < 14; ++i)
	{
		digitalValue[i] = 0;
	}
	for (int port = 1; port < 25; port++) {
		sprintf_s(portName, "\\\\.\\COM%d", port);
		SP = new Serial(portName);
		if (SP->IsConnected())
		{
			int dataLength = BUFFER_SIZE;
			ZeroMemory(InputData, sizeof(InputData));
			SP->ReadData(InputData, dataLength);
			BOOL first, second;
			first = InputData[0] >> 7;
			second = InputData[1] >> 7;
			if ((first || second) && !(first && second)) {
				PostMessage(hWnd, WM_SERIAL, NULL, FD_ACCEPT);
				return;
			}
		}

	}
	//MessageBox(NULL, L"not connect", NULL, NULL);
	PostMessage(hWnd, WM_SERIAL, NULL, FD_CLOSE);
}
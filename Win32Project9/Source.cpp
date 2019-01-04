#include <Windows.h>
#include <WinSock.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#pragma comment(lib,"wsock32.lib")

#define WM_SOCKET       (WM_USER+1)     // �\�P�b�g�p���b�Z�[�W
#define PORT            20000           // �ʐM�|�[�g�ԍ�
#define IDB_CONNECT     1000            // [�ڑ�]�{�^��
#define IDB_ACCEPT      1001            // [�ڑ��҂�]�{�^��
#define IDB_REJECT      1002            // [�ؒf]�{�^��
#define IDB_SEND        1003            // [���M]�{�^��
#define IDF_HOSTNAME    2000                // �z�X�g�����̓G�f�B�b�g�{�b�N�X
#define IDF_SENDMSG     2001            // ���M���b�Z�[�W���͗p�G�f�B�b�g�{�b�N�X
#define IDF_RECVMSG     2002            // ��M���b�Z�[�W�\���p�G�f�B�b�g�{�b�N�X
#define IDE_RECVMSG     3000            // ���b�Z�[�W��M�C�x���g
#define IDE_DECISION    4000           
#define WINDOW_W        1000         // �E�B���h�E�̕�
#define WINDOW_H        800         // �E�B���h�E�̍���
#define MAX_MESSAGE     128         // �z��̍ő�v�f��
#define ID_sinkei 1500
#define ID_chat 1600
#define ID_MYCHILD 1000
#define SERVER 1
#define CLIENT 2
#define MENU 10
#define CHAT 20
#define SINKEI 30
#define omote 1
#define ura 0
#define nasi -1

int USER = SERVER;
int JOUTAI = MENU;

typedef struct {
	int number;
	int joutai;
}card_t;

card_t ban[52];
int score1 = 0, score2 = 0;
int turn = 0, count = 0, iti, ni;

const RECT d = { 0, 0, 800, 600 };                  // �`��̈�(�������x���W, �������y���W, �E������x���W, �E������y���W)
const RECT d_main = { 100, 90, 900, 690 };


SOCKET socks[4] = { INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET };
int clients = 0, totalp = 0;

HINSTANCE hInstance;
SOCKET sock = INVALID_SOCKET;            // �\�P�b�g
SOCKET sv_sock = INVALID_SOCKET;            // �T�[�o�p�\�P�b�g
HOSTENT *phe;                       // HOSTENT�\����

BOOL InitApp(HINSTANCE hInstance, LPCTSTR szClassName, WNDPROC WndProc);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);   // �E�B���h�E�֐�
LRESULT CALLBACK chatProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK sinkeiProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OnPaint(HWND, UINT, WPARAM, LPARAM);
BOOL SockInit(HWND hWnd);                               // �\�P�b�g������
BOOL SockAccept(HWND hWnd);                             // �\�P�b�g�ڑ��҂�
BOOL SockConnect(HWND hWnd, LPCSTR host);               // �\�P�b�g�ڑ�
void WindowInit(HWND hWnd);                             // �E�B���h�E������
static HWND hWndRecvMSG;
static HWND hWndSendMSG;                    // ���M���b�Z�[�W���͗p�G�f�B�b�g�{�b�N�X
static HWND hWndRecvMSGT;
static HWND hWndSendMSGT;
static HWND clientnumWnd, hWndSend;                       // [���M]�{�^��
char message[10000] = "";
int x = 0, y = 0;


void shufful() {
	int a, b;
	card_t c;
	int i;
	srand((int)time(NULL));
	for (i = 0; i < 52; i++) {
		a = rand() % 52;
		b = rand() % 52;
		c = ban[a];
		ban[a] = ban[b];
		ban[b] = c;
	}
}

int check(int x, int y) {
	if (ban[0].joutai == ura)
		return 0;
	else
		return 1;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;                                          // �E�B���h�E�n���h��
	MSG  msg;                                           // ���b�Z�[�W

	hInstance = hInstance;                              //�O���[�o����

	InitApp(hInstance, "start", WindowProc);
	// �E�B���h�E����
	hWnd = CreateWindow(
		"start",                                // �E�B���h�E�N���X��
		"chat&game",                               // �E�B���h�E��
		WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU,          // �E�B���h�E����
		CW_USEDEFAULT,                              // �E�B���h�E�\���ʒu(X)
		CW_USEDEFAULT,                              // �E�B���h�E�\���ʒu(Y)
		WINDOW_W,                                   // �E�B���h�E�T�C�Y(X)
		WINDOW_H,                                   // �E�B���h�E�T�C�Y(Y)
		HWND_DESKTOP,                               // �e�E�B���h�E�n���h��
		NULL,
		hInstance,                                  // �C���X�^���X�n���h��
		NULL
		);

	// �E�B���h�E�\��
	ShowWindow(hWnd, nCmdShow);                         // �E�B���h�E�\�����[�h
	UpdateWindow(hWnd);                                 // �E�C���h�E�X�V

														// ���b�Z�[�W���[�v
	while (GetMessage(&msg, NULL, 0, 0)) {                // ���b�Z�[�W���擾
		TranslateMessage(&msg);
		DispatchMessage(&msg);                          // ���b�Z�[�W����
	}
	return (int)msg.wParam;                             // �v���O�����I��
}

BOOL InitApp(HINSTANCE hInstance, LPCTSTR szClassName, WNDPROC WndProc)
{
	WNDCLASSEX wc;                                      // �E�B���h�E�N���X

														//�E�B���h�E�N���X��`
	wc.hInstance = hInstance;                       // �C���X�^���X
	wc.lpszClassName = szClassName;                     // �N���X��
	wc.lpfnWndProc = WndProc;                      // �E�B���h�E�֐���
	wc.style = 0;                               // �N���X�X�^�C��
	wc.cbSize = sizeof(WNDCLASSEX);              // �\���̃T�C�Y
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // �A�C�R���n���h��
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);     // �X���[���A�C�R��
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // �}�E�X�|�C���^
	wc.lpszMenuName = NULL;                            // ���j���[(�Ȃ�)
	wc.cbClsExtra = 0;                               // �N���X�g�����
	wc.cbWndExtra = 0;                               // �E�B���h�E�g�����
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;           // �E�B���h�E�̔w�i�F
	return RegisterClassEx(&wc);                // �E�B���h�E�N���X�o�^

}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	static HWND hWndHost, hChdWnd,  decisionWnd;
	static HWND hWndConnect, hWndAccept;
	static HWND chatWnd,sinkeiWnd;
	static HWND hWndReject;
	int point;
	char buf[MAX_MESSAGE];                  // ��M���e���ꎞ�I�Ɋi�[����o�b�t�@


	switch (uMsg) {
	case WM_CREATE:     // �E�B���h�E���������ꂽ
						// ������\��
		CreateWindow("static", "Host Name",
			WS_CHILD | WS_VISIBLE, 10, 10, 100, 18,
			hWnd, NULL, hInstance, NULL);

		// �z�X�g�����͗p�G�f�B�b�g�{�b�N�X
		hWndHost = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "",
			WS_CHILD | WS_VISIBLE, 10, 30, 200, 25,
			hWnd, (HMENU)IDF_HOSTNAME, hInstance, NULL);
		// [�ڑ�]�{�^��
		hWndConnect = CreateWindow("button", "�ڑ�",
			WS_CHILD | WS_VISIBLE, 220, 30, 50, 25,
			hWnd, (HMENU)IDB_CONNECT, hInstance, NULL);
		// [�ڑ��҂�]�{�^��
		hWndAccept = CreateWindow("button", "�ڑ��҂�",
			WS_CHILD | WS_VISIBLE, 275, 30, 90, 25,
			hWnd, (HMENU)IDB_ACCEPT, hInstance, NULL);
		// [�ؒf]�{�^��
		hWndReject = CreateWindow("button", "�ؒf",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 370, 30, 50, 25,
			hWnd, (HMENU)IDB_REJECT, hInstance, NULL);
		//�`���b�g�{�^��
		chatWnd = CreateWindow("button", "�`���b�g",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 425, 30, 90, 25,
			hWnd, (HMENU)ID_chat, hInstance, NULL);
		//�_�o����{�^��
		sinkeiWnd = CreateWindow("button", "�_�o����",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 520, 30, 90, 25,
			hWnd, (HMENU)ID_sinkei, hInstance, NULL);
		//�g
		hChdWnd = CreateWindow("static", "",
			WS_CHILD | WS_BORDER | WS_VISIBLE, 100, 90, 800, 600,
			hWnd, NULL, hInstance, NULL);
		//�l�� 
		clientnumWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("1�l"),
			WS_CHILD | ES_READONLY | WS_VISIBLE, 730, 30, 150, 25,
			hWnd, NULL, hInstance, NULL);
		//�l������{�^��
		decisionWnd = CreateWindow("button", "�l������",
			WS_CHILD | WS_DISABLED | WS_VISIBLE, 885, 30, 90, 25,
			hWnd, (HMENU)IDE_DECISION, hInstance, NULL);

		SetFocus(hWndHost);     //�t�H�[�J�X�w��
		SockInit(hWnd);         // �\�P�b�g������

		return 0L;

	case WM_COMMAND:    // �{�^���������ꂽ
		switch (LOWORD(wP)) {
		case ID_chat:
			if (USER == SERVER) {
				int i;
				for (i = 0; i < clients; i++) {
					send(socks[i], "c", 1, 0);
				}
				clients = 0;
			}
			JOUTAI = CHAT;
			InitApp(hInstance, "chat", chatProc);
			hChdWnd = CreateWindow("chat", "",
				WS_CHILD | WS_BORDER, 100, 90, 800, 600,
				hWnd, (HMENU)ID_MYCHILD, hInstance, NULL);
			ShowWindow(hChdWnd, SW_SHOW);
			UpdateWindow(hChdWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			EnableWindow(chatWnd, FALSE);
			EnableWindow(sinkeiWnd, FALSE);
			return 0L;

		case ID_sinkei:
			if (USER == SERVER) {
				int i;
				for (i = 0; i < clients; i++) {
					send(socks[i], "s", 1, 0);
				}
				clients = 0;
			}
			char set[20];
			sprintf_s(set,"�v���C���[�F%d/%d",clients,totalp);
			SetWindowText(clientnumWnd, set);
			JOUTAI = SINKEI;
			InitApp(hInstance, "sinkei", sinkeiProc);
			hChdWnd = CreateWindow("sinkei", "",
				WS_CHILD | WS_BORDER, 100, 90, 800, 600,
				hWnd, (HMENU)ID_MYCHILD, hInstance, NULL);
			ShowWindow(hChdWnd, SW_SHOW);
			UpdateWindow(hChdWnd);
			InvalidateRect(hWnd, NULL, TRUE);
			EnableWindow(chatWnd, FALSE);
			EnableWindow(sinkeiWnd, FALSE);
			return 0L;

		case IDB_ACCEPT:    // [�ڑ��҂�]�{�^������(�T�[�o�[)
			if (SockAccept(hWnd)) {  // �ڑ��҂��v��
				return 0L;      // �ڑ��҂����s
			}
			EnableWindow(hWndConnect, FALSE);
			EnableWindow(hWndAccept, FALSE);
			EnableWindow(hWndReject, TRUE);
			EnableWindow(hWndHost, FALSE);    // [HostName]����		
			USER = SERVER;
			return 0L;

		case IDB_CONNECT:   // [�ڑ�]�{�^������(�N���C�A���g)
			char host[100];
			GetWindowText(hWndHost, host, sizeof(host));

			if (SockConnect(hWnd, host)) {   // �ڑ��v��
				SetFocus(hWndHost);     // �ڑ����s
				return 0L;
			}
			EnableWindow(hWndConnect, FALSE);
			EnableWindow(hWndAccept, FALSE);
			EnableWindow(hWndReject, TRUE);
			EnableWindow(hWndHost, FALSE);
			USER = CLIENT;
			return 0L;

		case IDB_REJECT:    // [�ؒf]�{�^������
			if (sock != INVALID_SOCKET) {    // �������N���C�A���g���Ȃ�\�P�b�g�����
				closesocket(sock);
				sock = INVALID_SOCKET;
			}
			if (sv_sock != INVALID_SOCKET) { // �������T�[�o���Ȃ�T�[�o�p�\�P�b�g�����
				closesocket(socks[0]);
				closesocket(socks[1]);
				closesocket(socks[2]);
				closesocket(socks[3]);
				closesocket(sv_sock);
				socks[0] = INVALID_SOCKET;
				socks[1] = INVALID_SOCKET;
				socks[2] = INVALID_SOCKET;
				socks[3] = INVALID_SOCKET;
				sv_sock = INVALID_SOCKET;
			}
			JOUTAI = MENU;
			SetWindowText(clientnumWnd, "1�l");
			phe = NULL;
			clients = 0;
			turn = 0;
			message[0] = '\0';
			DestroyWindow(hChdWnd);

			EnableWindow(hWndHost, TRUE);       // [HostName]�L��
			EnableWindow(hWndConnect, TRUE);    // [�ڑ�]    �L��
			EnableWindow(hWndAccept, TRUE);     // [�ڑ��҂�]�L��
			EnableWindow(hWndReject, FALSE);    // [�ؒf]    ����
			EnableWindow(chatWnd, FALSE);
			EnableWindow(sinkeiWnd, FALSE);
			EnableWindow(decisionWnd, FALSE);
			return 0L;

		case IDE_DECISION:
			EnableWindow(decisionWnd, FALSE);
			EnableWindow(chatWnd, TRUE);
			EnableWindow(sinkeiWnd, TRUE);
			totalp = clients+1;
		}

		return 0L;

	case WM_SOCKET:          // �񓯊��������b�Z�[�W
		if (WSAGETSELECTERROR(lP) != 0) { return 0L; }

		switch (WSAGETSELECTEVENT(lP)) {
		case FD_READ:       //���b�Z�[�W��M
			if (JOUTAI == MENU) {
				if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
					if (buf[0] == 'c') {
						SendMessage(hWnd, WM_COMMAND, ID_chat, 0);
					}
					if (buf[0] == 'p') {
						clients++;
						char person[5];
						sprintf_s(person, "%d�l", buf[1]-'0' + 1);
						SetWindowText(clientnumWnd, person);
						totalp = buf[1] - '0' + 1;
					}
					if (buf[0] == 's') {
						SendMessage(hWnd, WM_COMMAND, ID_sinkei, 0);
					}
				}

			}

			if (JOUTAI == CHAT) {
				if (USER == CLIENT) {
					if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) { // ��M�ł����Ȃ�
						strcat_s(buf, "\r\n");
						strcat_s(message, buf);
						SetWindowText(hWndRecvMSG, message);
					}
				}
				if (USER == SERVER) {
					if (recv(socks[0], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						strcat_s(buf, "\r\n");
						strcat_s(message, buf);
						SetWindowText(hWndRecvMSG, message);
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[1], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						strcat_s(buf, "\r\n");
						strcat_s(message, buf);
						SetWindowText(hWndRecvMSG, message);
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[2], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						strcat_s(buf, "\r\n");
						strcat_s(message, buf);
						SetWindowText(hWndRecvMSG, message);
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[3], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						strcat_s(buf, "\r\n");
						strcat_s(message, buf);
						SetWindowText(hWndRecvMSG, message);
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
					}
				}
				return 0L;
			}

			if (JOUTAI == SINKEI) {
				if (USER == SERVER) {
					if (recv(socks[0], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[1], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[2], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[3], buf, strlen(buf) + 1, 0);
					}
					if (recv(socks[3], buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
						send(socks[0], buf, strlen(buf) + 1, 0);
						send(socks[1], buf, strlen(buf) + 1, 0);
						send(socks[2], buf, strlen(buf) + 1, 0);
					}
				}
				else {
					if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR);
				}

				point = atoi(buf);
				x = point / 1000;
				y = point % 1000;

				if (count % 3 == 2) {
					if (ban[iti].number % 13 == ban[ni].number % 13) {
						ban[iti].joutai = nasi;
						ban[ni].joutai = nasi;
						if (turn % 2 == 0) {
							score1 += 2;
						}
						else if (turn % 2 == 1) {
							score2 += 2;
						}
						count++;
					}
					else {
						ban[iti].joutai = ura;
						ban[ni].joutai = ura;
						count++;
						turn++;
					}
					InvalidateRect(hWnd, &d_main, TRUE);
					return 0L;
				}

				if (20 < x % 60 && 40 < y % 150 && y % 150 < 120) {
					x /= 60;
					y /= 150;

					if (ban[x + y * 13].joutai == ura) {
						if (count % 3 == 0) {
							ban[x + y * 13].joutai = omote;
							iti = x + y * 13;
							count++;
						}
						else if (count % 3 == 1) {
							ban[x + y * 13].joutai = omote;
							ni = x + y * 13;
							count++;
						}
						InvalidateRect(hWnd, &d_main, TRUE);
						return 0;
					}
				}


			}

			return 0L;

		case FD_ACCEPT:     // �ڑ��҂������ʒm
		{
			if (JOUTAI == MENU) {
				SOCKADDR_IN cl_sin;
				int len = sizeof(cl_sin);

				socks[clients++] = accept(sv_sock, (LPSOCKADDR)&cl_sin, &len);
				EnableWindow(decisionWnd, TRUE);
				int i;
				sprintf_s(buf,"p%d",clients);
				for (i = 0; i < clients; i++) {
					send(socks[i], buf, 3, 0);
				}
				if (socks[clients - 1] == INVALID_SOCKET) {
					MessageBox(hWnd, "Accepting connection failed",
						"Error", MB_OK | MB_ICONEXCLAMATION);
					closesocket(sv_sock);
					sv_sock = INVALID_SOCKET;
					return 0L;
				}
				// �񓯊����[�h (��M���ؒf�j
				if (WSAAsyncSelect(socks[clients - 1], hWnd, WM_SOCKET, FD_READ | FD_CLOSE)
					== SOCKET_ERROR) {
					// �ڑ��Ɏ��s�����珉����Ԃɖ߂�
					MessageBox(hWnd, "WSAAsyncSelect() failed",
						"Error", MB_OK | MB_ICONEXCLAMATION);
					return 0L;
				}
				char person[5];
				sprintf_s(person, "%d�l", clients + 1);
				SetWindowText(clientnumWnd, person);
				return 0L;
			}
		}/* end of case FD_ACCEPT: */

		case FD_CONNECT:    // �ڑ������ʒm
							// �񓯊����[�h (��M���ؒf)
			if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
				// �ڑ��Ɏ��s�����珉����Ԃɖ߂�
				MessageBox(hWnd, "WSAAsyncSelect() failed",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				return 0L;
			}
			//turn++;
			return 0L;

		case FD_CLOSE:      // �ؒf���ꂽ
			MessageBox(hWnd, "�ؒf����܂����B",
				"Information", MB_OK | MB_ICONINFORMATION);
			SendMessage(hWnd, WM_COMMAND, IDB_REJECT, 0); // �ؒf�������s
			return 0L;
		}/* end of switch (WSAGETSELECTEVENT(lP)) */
		return 0L;


	case WM_DESTROY:    // �E�B���h�E���j�����ꂽ
		closesocket(sock);
		PostQuitMessage(0);
		return 0L;



	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // �W�����b�Z�[�W����
	}/* end of switch (uMsg) */
}


////////////////////////////////////////////////////////////////////////////////
//
//  �\�P�b�g����������
//
BOOL SockInit(HWND hWnd)
{
	WSADATA wsa;
	int ret;
	char ret_buf[80];

	ret = WSAStartup(MAKEWORD(1, 1), &wsa);

	if (ret != 0) {
		wsprintf(ret_buf, "%d is the err", ret);
		MessageBox(hWnd, ret_buf, "Error", MB_OK | MB_ICONSTOP);
		exit(-1);
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
//  �\�P�b�g�ڑ� (�N���C�A���g��)
//
BOOL SockConnect(HWND hWnd, LPCSTR host)
{
	SOCKADDR_IN cl_sin; // SOCKADDR_IN�\����

						// �\�P�b�g���J��
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {        // �\�P�b�g�쐬���s
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&cl_sin, 0x00, sizeof(cl_sin)); // �\���̏�����
	cl_sin.sin_family = AF_INET;           // �C���^�[�l�b�g
	cl_sin.sin_port = htons(PORT);       // �|�[�g�ԍ��w��

	phe = gethostbyname(host); // �A�h���X�擾

	if (phe == NULL) {
		MessageBox(hWnd, "gethostbyname() failed.",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}
	memcpy(&cl_sin.sin_addr, phe->h_addr, phe->h_length);

	// �񓯊����[�h (�ڑ�)
	if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_CONNECT) == SOCKET_ERROR) {
		closesocket(sock);
		sock = INVALID_SOCKET;
		MessageBox(hWnd, "WSAAsyncSelect() failed",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// �ڑ�����
	if (connect(sock, (LPSOCKADDR)&cl_sin, sizeof(cl_sin)) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			closesocket(sock);
			sock = INVALID_SOCKET;
			MessageBox(hWnd, "connect() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
			return TRUE;
		}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
//  �ڑ��҂� (�T�[�o��)
//
BOOL SockAccept(HWND hWnd)
{
	SOCKADDR_IN sv_sin;         // SOCKADDR_IN�\����

								// �T�[�o�p�\�P�b�g
	sv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sv_sock == INVALID_SOCKET) { // �\�P�b�g�쐬���s
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&sv_sin, 0x00, sizeof(sv_sin));      // �\���̏�����
	sv_sin.sin_family = AF_INET;           // �C���^�[�l�b�g
	sv_sin.sin_port = htons(PORT);       // �|�[�g�ԍ��w��
	sv_sin.sin_addr.s_addr = htonl(INADDR_ANY); // �A�h���X�w��

	if (bind(sv_sock, (LPSOCKADDR)&sv_sin, sizeof(sv_sin)) == SOCKET_ERROR) {
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "bind() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if (listen(sv_sock, 5) == SOCKET_ERROR) {
		// �ڑ��҂����s
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "listen() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// �񓯊��������[�h (�ڑ��҂�)
	if (WSAAsyncSelect(sv_sock, hWnd, WM_SOCKET, FD_ACCEPT) == SOCKET_ERROR) {
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "WSAAsyncSelect() failed",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK chatProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	char buf[MAX_MESSAGE];                  // ���M���e���ꎞ�I�Ɋi�[����o�b�t�@

	switch (uMsg) {
	case WM_CREATE:

		hWndRecvMSGT = CreateWindow(TEXT("static"), TEXT("Message List"), WS_CHILD | WS_VISIBLE,
			10, 20, 200, 18, hWnd, NULL, hInstance, NULL);

		hWndRecvMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, 50, 50, 700, 400,
			hWnd, (HMENU)IDF_RECVMSG, hInstance, NULL);

		hWndSendMSGT = CreateWindow(TEXT("static"), TEXT("Send Message"), WS_CHILD | WS_VISIBLE,
			10, 470, 200, 18, hWnd, NULL, hInstance, NULL);

		hWndSend = CreateWindow("button", "���M",
			WS_CHILD | WS_VISIBLE, 655, 500, 90, 25,
			hWnd, (HMENU)IDB_SEND, hInstance, NULL);

		hWndSendMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE, 50, 500, 600, 30,
			hWnd, (HMENU)IDF_SENDMSG, hInstance, NULL);

		SetFocus(hWndSendMSG);

		return 0;

	case WM_COMMAND:    // �{�^���������ꂽ
		switch (LOWORD(wP)) {

		case IDB_SEND:      // [���M]�{�^������
			GetWindowText(hWndSendMSG, buf, sizeof(buf) - 1);     // ���M���b�Z�[�W���͗��̓��e���擾

			if (USER == SERVER) {
				send(socks[0], buf, strlen(buf) + 1, 0);
				send(socks[1], buf, strlen(buf) + 1, 0);
				send(socks[2], buf, strlen(buf) + 1, 0);
				send(socks[3], buf, strlen(buf) + 1, 0);
			}
			if (USER == CLIENT) {
				send(sock, buf, strlen(buf) + 1, 0);
			}
			strcat_s(buf, "\r\n");
			strcat_s(message, buf);
			SetWindowText(hWndRecvMSG, message);
			SetWindowText(hWndSendMSG, TEXT(""));    // ���M���b�Z�[�W���͗p�G�f�B�b�g�{�b�N�X����ɂ���
			SetFocus(hWndSendMSG);          // �t�H�[�J�X�w��
			return 0L;

		} /* end of switch (LOWORD(wP)) */
		return 0L;

	case WM_DESTROY:    // �E�B���h�E���j�����ꂽ
		return 0L;

	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // �W�����b�Z�[�W����
	}
}

LRESULT CALLBACK sinkeiProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	int i;
	char buf[MAX_MESSAGE];                  // ��M���e���ꎞ�I�Ɋi�[����o�b�t�@

	switch (uMsg) {
	case WM_CREATE:     // �E�B���h�E���������ꂽ
		for (i = 0; i < 52; i++) {
			ban[i].number = i;
			ban[i].joutai = ura;
		}
		shufful();
		return 0L;

	case WM_LBUTTONDOWN:    // �}�E�X���{�^���������ꂽ
		if (JOUTAI == SINKEI) {
			x = (LOWORD(lP));
			y = (HIWORD(lP));

			if (x > 800 || x < 0 || y < 0 || y>600)break;

			if (turn % totalp == clients) {

				sprintf_s(buf, "%03d%03d\0", x, y);
				if (USER == SERVER) {
					for (i = 0;i<totalp-1 ;i++)
					if (send(socks[i], buf, strlen(buf) + 1, 0) == SOCKET_ERROR) {    // ���M����
						MessageBox(hWnd, TEXT("sending failed"), TEXT("Error"),       // ���M�Ɏ��s������G���[��\��
							MB_OK | MB_ICONEXCLAMATION);
					}
				}
				if (USER == CLIENT) {
					if (send(sock, buf, strlen(buf) + 1, 0) == SOCKET_ERROR) {    // ���M����
						MessageBox(hWnd, TEXT("sending failed"), TEXT("Error"),       // ���M�Ɏ��s������G���[��\��
							MB_OK | MB_ICONEXCLAMATION);
					}
				}

				if (count % 3 == 2) {
					if (ban[iti].number % 13 == ban[ni].number % 13) {
						ban[iti].joutai = nasi;
						ban[ni].joutai = nasi;
						if (turn % 2 == 0) {
							score1 += 2;
						}
						else if (turn % 2 == 1) {
							score2 += 2;
						}
						count++;
					}
					else {
						ban[iti].joutai = ura;
						ban[ni].joutai = ura;
						count++;
						turn++;
					}
					InvalidateRect(hWnd, &d, TRUE);
					return 0L;
				}

				if (20 < x % 60 && 40 < y % 150 && y % 150 < 120) {
					x /= 60;
					y /= 150;

					if (ban[x + y * 13].joutai == ura) {
						if (count % 3 == 0) {
							ban[x + y * 13].joutai = omote;
							iti = x + y * 13;
							count++;
						}
						else if (count % 3 == 1) {
							ban[x + y * 13].joutai = omote;
							ni = x + y * 13;
							count++;
						}
						InvalidateRect(hWnd, &d, TRUE);
						return 0;
					}
				}

			}
		}
		return 0;

	case WM_PAINT:      // �ĕ`��
		return OnPaint(hWnd, uMsg, wP, lP);

	case WM_DESTROY:    // �E�B���h�E���j�����ꂽ
		return 0L;

	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // �W�����b�Z�[�W����
	}/* end of switch (uMsg) */
}

LRESULT CALLBACK OnPaint(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	HDC hDC;                                    // �f�o�C�X�R���e�L�X�g
	HBRUSH hBrushOld, hBrushBlack;              // ���u���V�ۑ��p
	HPEN hPenOld, hPenBlack;          // �y���E�n���h��
	HFONT hFont;
	PAINTSTRUCT ps;                             // PAINTSTRUCT�\����
	int i, j, num;
	char num_str[3];
	LPTSTR mark = "";
	char score1_str[7];
	char score2_str[7];

	hDC = BeginPaint(hWnd, &ps);                    // �`��J�n

	SetBkMode(hDC, TRANSPARENT);
	hFont = CreateFont(
		12, 0, 0, 0, FW_BOLD, TRUE, TRUE, FALSE,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		VARIABLE_PITCH | FF_ROMAN, NULL
		);
	SelectObject(hDC, hFont);

	hBrushBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);          // ���u���V�擾 ���@HBRUSH�^�ɃL���X�g
	hPenBlack = (HPEN)GetStockObject(BLACK_PEN);              // ���y���擾   ���@HPEN�^�ɃL���X�g
	hBrushOld = (HBRUSH)SelectObject(hDC, hBrushBlack);        // �u���V�I�� ���@HBRUSH�^�ɃL���X�g
	hPenOld = (HPEN)SelectObject(hDC, hPenBlack);           // ���y���I��   ���@HPEN�^�ɃL���X�g

	if (turn % totalp == clients) {
		TextOut(hDC, 0, 0, "���Ȃ��̔Ԃł�", 14);
	}
	else {
		TextOut(hDC, 0, 0, "����̔Ԃł�", 12);
	}
	sprintf_s(score1_str, "1:%2d��", score1);
	sprintf_s(score2_str, "2:%2d��", score2);
	TextOut(hDC, 100, 0, score1_str, 7);
	TextOut(hDC, 200, 0, score2_str, 7);

	if (score1 + score2 == 52) {
		TextOut(hDC, 300, 0, TEXT("�I��"), 4);
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 13; j++) {
			if (ban[i * 13 + j].joutai == ura) {
				Rectangle(hDC,
					20 + j * 60, 30 + i * 150,
					60 + j * 60, 110 + i * 150);
			}
			else if (ban[i * 13 + j].joutai == nasi) {
			}
			else {
				if (ban[(i * 13 + j)].number / 13 == 0) {
					mark = "�X�y�[�h";
				}
				else if (ban[(i * 13 + j)].number / 13 == 1) {
					mark = "�n�[�g";
				}
				else if (ban[(i * 13 + j)].number / 13 == 2) {
					mark = "�N���[�o�[";
				}
				else if (ban[(i * 13 + j)].number / 13 == 3) {
					mark = "�_�C��";
				}
				num = (ban[i * 13 + j].number % 13) + 1;
				sprintf_s(num_str, "%2d", num);
				TextOut(hDC, 20 + j * 60, 50 + i * 150, mark, lstrlen(mark));
				TextOut(hDC, 30 + j * 60, 90 + i * 150, num_str, lstrlen(num_str));
			}
		}
	}
	EndPaint(hWnd, &ps);            // �`��I��
	return 0L;
}
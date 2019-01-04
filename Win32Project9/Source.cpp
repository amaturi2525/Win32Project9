#include <Windows.h>
#include <WinSock.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#pragma comment(lib,"wsock32.lib")

#define WM_SOCKET       (WM_USER+1)     // ソケット用メッセージ
#define PORT            20000           // 通信ポート番号
#define IDB_CONNECT     1000            // [接続]ボタン
#define IDB_ACCEPT      1001            // [接続待ち]ボタン
#define IDB_REJECT      1002            // [切断]ボタン
#define IDB_SEND        1003            // [送信]ボタン
#define IDF_HOSTNAME    2000                // ホスト名入力エディットボックス
#define IDF_SENDMSG     2001            // 送信メッセージ入力用エディットボックス
#define IDF_RECVMSG     2002            // 受信メッセージ表示用エディットボックス
#define IDE_RECVMSG     3000            // メッセージ受信イベント
#define IDE_DECISION    4000           
#define WINDOW_W        1000         // ウィンドウの幅
#define WINDOW_H        800         // ウィンドウの高さ
#define MAX_MESSAGE     128         // 配列の最大要素数
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

const RECT d = { 0, 0, 800, 600 };                  // 描画領域(左上隅のx座標, 左上隅のy座標, 右下隅のx座標, 右下隅のy座標)
const RECT d_main = { 100, 90, 900, 690 };


SOCKET socks[4] = { INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET,INVALID_SOCKET };
int clients = 0, totalp = 0;

HINSTANCE hInstance;
SOCKET sock = INVALID_SOCKET;            // ソケット
SOCKET sv_sock = INVALID_SOCKET;            // サーバ用ソケット
HOSTENT *phe;                       // HOSTENT構造体

BOOL InitApp(HINSTANCE hInstance, LPCTSTR szClassName, WNDPROC WndProc);
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);   // ウィンドウ関数
LRESULT CALLBACK chatProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK sinkeiProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OnPaint(HWND, UINT, WPARAM, LPARAM);
BOOL SockInit(HWND hWnd);                               // ソケット初期化
BOOL SockAccept(HWND hWnd);                             // ソケット接続待ち
BOOL SockConnect(HWND hWnd, LPCSTR host);               // ソケット接続
void WindowInit(HWND hWnd);                             // ウィンドウ初期化
static HWND hWndRecvMSG;
static HWND hWndSendMSG;                    // 送信メッセージ入力用エディットボックス
static HWND hWndRecvMSGT;
static HWND hWndSendMSGT;
static HWND clientnumWnd, hWndSend;                       // [送信]ボタン
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
	HWND hWnd;                                          // ウィンドウハンドル
	MSG  msg;                                           // メッセージ

	hInstance = hInstance;                              //グローバル化

	InitApp(hInstance, "start", WindowProc);
	// ウィンドウ生成
	hWnd = CreateWindow(
		"start",                                // ウィンドウクラス名
		"chat&game",                               // ウィンドウ名
		WS_DLGFRAME | WS_VISIBLE | WS_SYSMENU,          // ウィンドウ属性
		CW_USEDEFAULT,                              // ウィンドウ表示位置(X)
		CW_USEDEFAULT,                              // ウィンドウ表示位置(Y)
		WINDOW_W,                                   // ウィンドウサイズ(X)
		WINDOW_H,                                   // ウィンドウサイズ(Y)
		HWND_DESKTOP,                               // 親ウィンドウハンドル
		NULL,
		hInstance,                                  // インスタンスハンドル
		NULL
		);

	// ウィンドウ表示
	ShowWindow(hWnd, nCmdShow);                         // ウィンドウ表示モード
	UpdateWindow(hWnd);                                 // ウインドウ更新

														// メッセージループ
	while (GetMessage(&msg, NULL, 0, 0)) {                // メッセージを取得
		TranslateMessage(&msg);
		DispatchMessage(&msg);                          // メッセージ送る
	}
	return (int)msg.wParam;                             // プログラム終了
}

BOOL InitApp(HINSTANCE hInstance, LPCTSTR szClassName, WNDPROC WndProc)
{
	WNDCLASSEX wc;                                      // ウィンドウクラス

														//ウィンドウクラス定義
	wc.hInstance = hInstance;                       // インスタンス
	wc.lpszClassName = szClassName;                     // クラス名
	wc.lpfnWndProc = WndProc;                      // ウィンドウ関数名
	wc.style = 0;                               // クラススタイル
	wc.cbSize = sizeof(WNDCLASSEX);              // 構造体サイズ
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // アイコンハンドル
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);     // スモールアイコン
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // マウスポインタ
	wc.lpszMenuName = NULL;                            // メニュー(なし)
	wc.cbClsExtra = 0;                               // クラス拡張情報
	wc.cbWndExtra = 0;                               // ウィンドウ拡張情報
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;           // ウィンドウの背景色
	return RegisterClassEx(&wc);                // ウィンドウクラス登録

}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	static HWND hWndHost, hChdWnd,  decisionWnd;
	static HWND hWndConnect, hWndAccept;
	static HWND chatWnd,sinkeiWnd;
	static HWND hWndReject;
	int point;
	char buf[MAX_MESSAGE];                  // 受信内容を一時的に格納するバッファ


	switch (uMsg) {
	case WM_CREATE:     // ウィンドウが生成された
						// 文字列表示
		CreateWindow("static", "Host Name",
			WS_CHILD | WS_VISIBLE, 10, 10, 100, 18,
			hWnd, NULL, hInstance, NULL);

		// ホスト名入力用エディットボックス
		hWndHost = CreateWindowEx(WS_EX_CLIENTEDGE, "edit", "",
			WS_CHILD | WS_VISIBLE, 10, 30, 200, 25,
			hWnd, (HMENU)IDF_HOSTNAME, hInstance, NULL);
		// [接続]ボタン
		hWndConnect = CreateWindow("button", "接続",
			WS_CHILD | WS_VISIBLE, 220, 30, 50, 25,
			hWnd, (HMENU)IDB_CONNECT, hInstance, NULL);
		// [接続待ち]ボタン
		hWndAccept = CreateWindow("button", "接続待ち",
			WS_CHILD | WS_VISIBLE, 275, 30, 90, 25,
			hWnd, (HMENU)IDB_ACCEPT, hInstance, NULL);
		// [切断]ボタン
		hWndReject = CreateWindow("button", "切断",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 370, 30, 50, 25,
			hWnd, (HMENU)IDB_REJECT, hInstance, NULL);
		//チャットボタン
		chatWnd = CreateWindow("button", "チャット",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 425, 30, 90, 25,
			hWnd, (HMENU)ID_chat, hInstance, NULL);
		//神経衰弱ボタン
		sinkeiWnd = CreateWindow("button", "神経衰弱",
			WS_CHILD | WS_VISIBLE | WS_DISABLED, 520, 30, 90, 25,
			hWnd, (HMENU)ID_sinkei, hInstance, NULL);
		//枠
		hChdWnd = CreateWindow("static", "",
			WS_CHILD | WS_BORDER | WS_VISIBLE, 100, 90, 800, 600,
			hWnd, NULL, hInstance, NULL);
		//人数 
		clientnumWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT("1人"),
			WS_CHILD | ES_READONLY | WS_VISIBLE, 730, 30, 150, 25,
			hWnd, NULL, hInstance, NULL);
		//人数決定ボタン
		decisionWnd = CreateWindow("button", "人数決定",
			WS_CHILD | WS_DISABLED | WS_VISIBLE, 885, 30, 90, 25,
			hWnd, (HMENU)IDE_DECISION, hInstance, NULL);

		SetFocus(hWndHost);     //フォーカス指定
		SockInit(hWnd);         // ソケット初期化

		return 0L;

	case WM_COMMAND:    // ボタンが押された
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
			sprintf_s(set,"プレイヤー：%d/%d",clients,totalp);
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

		case IDB_ACCEPT:    // [接続待ち]ボタン押下(サーバー)
			if (SockAccept(hWnd)) {  // 接続待ち要求
				return 0L;      // 接続待ち失敗
			}
			EnableWindow(hWndConnect, FALSE);
			EnableWindow(hWndAccept, FALSE);
			EnableWindow(hWndReject, TRUE);
			EnableWindow(hWndHost, FALSE);    // [HostName]無効		
			USER = SERVER;
			return 0L;

		case IDB_CONNECT:   // [接続]ボタン押下(クライアント)
			char host[100];
			GetWindowText(hWndHost, host, sizeof(host));

			if (SockConnect(hWnd, host)) {   // 接続要求
				SetFocus(hWndHost);     // 接続失敗
				return 0L;
			}
			EnableWindow(hWndConnect, FALSE);
			EnableWindow(hWndAccept, FALSE);
			EnableWindow(hWndReject, TRUE);
			EnableWindow(hWndHost, FALSE);
			USER = CLIENT;
			return 0L;

		case IDB_REJECT:    // [切断]ボタン押下
			if (sock != INVALID_SOCKET) {    // 自分がクライアント側ならソケットを閉じる
				closesocket(sock);
				sock = INVALID_SOCKET;
			}
			if (sv_sock != INVALID_SOCKET) { // 自分がサーバ側ならサーバ用ソケットを閉じる
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
			SetWindowText(clientnumWnd, "1人");
			phe = NULL;
			clients = 0;
			turn = 0;
			message[0] = '\0';
			DestroyWindow(hChdWnd);

			EnableWindow(hWndHost, TRUE);       // [HostName]有効
			EnableWindow(hWndConnect, TRUE);    // [接続]    有効
			EnableWindow(hWndAccept, TRUE);     // [接続待ち]有効
			EnableWindow(hWndReject, FALSE);    // [切断]    無効
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

	case WM_SOCKET:          // 非同期処理メッセージ
		if (WSAGETSELECTERROR(lP) != 0) { return 0L; }

		switch (WSAGETSELECTEVENT(lP)) {
		case FD_READ:       //メッセージ受信
			if (JOUTAI == MENU) {
				if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) {
					if (buf[0] == 'c') {
						SendMessage(hWnd, WM_COMMAND, ID_chat, 0);
					}
					if (buf[0] == 'p') {
						clients++;
						char person[5];
						sprintf_s(person, "%d人", buf[1]-'0' + 1);
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
					if (recv(sock, buf, sizeof(buf) - 1, 0) != SOCKET_ERROR) { // 受信できたなら
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

		case FD_ACCEPT:     // 接続待ち完了通知
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
				// 非同期モード (受信＆切断）
				if (WSAAsyncSelect(socks[clients - 1], hWnd, WM_SOCKET, FD_READ | FD_CLOSE)
					== SOCKET_ERROR) {
					// 接続に失敗したら初期状態に戻す
					MessageBox(hWnd, "WSAAsyncSelect() failed",
						"Error", MB_OK | MB_ICONEXCLAMATION);
					return 0L;
				}
				char person[5];
				sprintf_s(person, "%d人", clients + 1);
				SetWindowText(clientnumWnd, person);
				return 0L;
			}
		}/* end of case FD_ACCEPT: */

		case FD_CONNECT:    // 接続完了通知
							// 非同期モード (受信＆切断)
			if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
				// 接続に失敗したら初期状態に戻す
				MessageBox(hWnd, "WSAAsyncSelect() failed",
					"Error", MB_OK | MB_ICONEXCLAMATION);
				return 0L;
			}
			//turn++;
			return 0L;

		case FD_CLOSE:      // 切断された
			MessageBox(hWnd, "切断されました。",
				"Information", MB_OK | MB_ICONINFORMATION);
			SendMessage(hWnd, WM_COMMAND, IDB_REJECT, 0); // 切断処理発行
			return 0L;
		}/* end of switch (WSAGETSELECTEVENT(lP)) */
		return 0L;


	case WM_DESTROY:    // ウィンドウが破棄された
		closesocket(sock);
		PostQuitMessage(0);
		return 0L;



	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // 標準メッセージ処理
	}/* end of switch (uMsg) */
}


////////////////////////////////////////////////////////////////////////////////
//
//  ソケット初期化処理
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
//  ソケット接続 (クライアント側)
//
BOOL SockConnect(HWND hWnd, LPCSTR host)
{
	SOCKADDR_IN cl_sin; // SOCKADDR_IN構造体

						// ソケットを開く
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {        // ソケット作成失敗
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&cl_sin, 0x00, sizeof(cl_sin)); // 構造体初期化
	cl_sin.sin_family = AF_INET;           // インターネット
	cl_sin.sin_port = htons(PORT);       // ポート番号指定

	phe = gethostbyname(host); // アドレス取得

	if (phe == NULL) {
		MessageBox(hWnd, "gethostbyname() failed.",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}
	memcpy(&cl_sin.sin_addr, phe->h_addr, phe->h_length);

	// 非同期モード (接続)
	if (WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_CONNECT) == SOCKET_ERROR) {
		closesocket(sock);
		sock = INVALID_SOCKET;
		MessageBox(hWnd, "WSAAsyncSelect() failed",
			"Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// 接続処理
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
//  接続待ち (サーバ側)
//
BOOL SockAccept(HWND hWnd)
{
	SOCKADDR_IN sv_sin;         // SOCKADDR_IN構造体

								// サーバ用ソケット
	sv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sv_sock == INVALID_SOCKET) { // ソケット作成失敗
		MessageBox(hWnd, "Socket() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	memset(&sv_sin, 0x00, sizeof(sv_sin));      // 構造体初期化
	sv_sin.sin_family = AF_INET;           // インターネット
	sv_sin.sin_port = htons(PORT);       // ポート番号指定
	sv_sin.sin_addr.s_addr = htonl(INADDR_ANY); // アドレス指定

	if (bind(sv_sock, (LPSOCKADDR)&sv_sin, sizeof(sv_sin)) == SOCKET_ERROR) {
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "bind() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	if (listen(sv_sock, 5) == SOCKET_ERROR) {
		// 接続待ち失敗
		closesocket(sv_sock);
		sv_sock = INVALID_SOCKET;
		MessageBox(hWnd, "listen() failed", "Error", MB_OK | MB_ICONEXCLAMATION);
		return TRUE;
	}

	// 非同期処理モード (接続待ち)
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
	char buf[MAX_MESSAGE];                  // 送信内容を一時的に格納するバッファ

	switch (uMsg) {
	case WM_CREATE:

		hWndRecvMSGT = CreateWindow(TEXT("static"), TEXT("Message List"), WS_CHILD | WS_VISIBLE,
			10, 20, 200, 18, hWnd, NULL, hInstance, NULL);

		hWndRecvMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY, 50, 50, 700, 400,
			hWnd, (HMENU)IDF_RECVMSG, hInstance, NULL);

		hWndSendMSGT = CreateWindow(TEXT("static"), TEXT("Send Message"), WS_CHILD | WS_VISIBLE,
			10, 470, 200, 18, hWnd, NULL, hInstance, NULL);

		hWndSend = CreateWindow("button", "送信",
			WS_CHILD | WS_VISIBLE, 655, 500, 90, 25,
			hWnd, (HMENU)IDB_SEND, hInstance, NULL);

		hWndSendMSG = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("edit"), TEXT(""),
			WS_CHILD | WS_VISIBLE | ES_MULTILINE, 50, 500, 600, 30,
			hWnd, (HMENU)IDF_SENDMSG, hInstance, NULL);

		SetFocus(hWndSendMSG);

		return 0;

	case WM_COMMAND:    // ボタンが押された
		switch (LOWORD(wP)) {

		case IDB_SEND:      // [送信]ボタン押下
			GetWindowText(hWndSendMSG, buf, sizeof(buf) - 1);     // 送信メッセージ入力欄の内容を取得

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
			SetWindowText(hWndSendMSG, TEXT(""));    // 送信メッセージ入力用エディットボックスを空にする
			SetFocus(hWndSendMSG);          // フォーカス指定
			return 0L;

		} /* end of switch (LOWORD(wP)) */
		return 0L;

	case WM_DESTROY:    // ウィンドウが破棄された
		return 0L;

	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // 標準メッセージ処理
	}
}

LRESULT CALLBACK sinkeiProc(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	int i;
	char buf[MAX_MESSAGE];                  // 受信内容を一時的に格納するバッファ

	switch (uMsg) {
	case WM_CREATE:     // ウィンドウが生成された
		for (i = 0; i < 52; i++) {
			ban[i].number = i;
			ban[i].joutai = ura;
		}
		shufful();
		return 0L;

	case WM_LBUTTONDOWN:    // マウス左ボタンが押された
		if (JOUTAI == SINKEI) {
			x = (LOWORD(lP));
			y = (HIWORD(lP));

			if (x > 800 || x < 0 || y < 0 || y>600)break;

			if (turn % totalp == clients) {

				sprintf_s(buf, "%03d%03d\0", x, y);
				if (USER == SERVER) {
					for (i = 0;i<totalp-1 ;i++)
					if (send(socks[i], buf, strlen(buf) + 1, 0) == SOCKET_ERROR) {    // 送信処理
						MessageBox(hWnd, TEXT("sending failed"), TEXT("Error"),       // 送信に失敗したらエラーを表示
							MB_OK | MB_ICONEXCLAMATION);
					}
				}
				if (USER == CLIENT) {
					if (send(sock, buf, strlen(buf) + 1, 0) == SOCKET_ERROR) {    // 送信処理
						MessageBox(hWnd, TEXT("sending failed"), TEXT("Error"),       // 送信に失敗したらエラーを表示
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

	case WM_PAINT:      // 再描画
		return OnPaint(hWnd, uMsg, wP, lP);

	case WM_DESTROY:    // ウィンドウが破棄された
		return 0L;

	default:
		return DefWindowProc(hWnd, uMsg, wP, lP);  // 標準メッセージ処理
	}/* end of switch (uMsg) */
}

LRESULT CALLBACK OnPaint(HWND hWnd, UINT uMsg, WPARAM wP, LPARAM lP)
{
	HDC hDC;                                    // デバイスコンテキスト
	HBRUSH hBrushOld, hBrushBlack;              // 旧ブラシ保存用
	HPEN hPenOld, hPenBlack;          // ペン・ハンドル
	HFONT hFont;
	PAINTSTRUCT ps;                             // PAINTSTRUCT構造体
	int i, j, num;
	char num_str[3];
	LPTSTR mark = "";
	char score1_str[7];
	char score2_str[7];

	hDC = BeginPaint(hWnd, &ps);                    // 描画開始

	SetBkMode(hDC, TRANSPARENT);
	hFont = CreateFont(
		12, 0, 0, 0, FW_BOLD, TRUE, TRUE, FALSE,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		VARIABLE_PITCH | FF_ROMAN, NULL
		);
	SelectObject(hDC, hFont);

	hBrushBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);          // 黒ブラシ取得 →　HBRUSH型にキャスト
	hPenBlack = (HPEN)GetStockObject(BLACK_PEN);              // 黒ペン取得   →　HPEN型にキャスト
	hBrushOld = (HBRUSH)SelectObject(hDC, hBrushBlack);        // 青ブラシ選択 →　HBRUSH型にキャスト
	hPenOld = (HPEN)SelectObject(hDC, hPenBlack);           // 黒ペン選択   →　HPEN型にキャスト

	if (turn % totalp == clients) {
		TextOut(hDC, 0, 0, "あなたの番です", 14);
	}
	else {
		TextOut(hDC, 0, 0, "相手の番です", 12);
	}
	sprintf_s(score1_str, "1:%2d枚", score1);
	sprintf_s(score2_str, "2:%2d枚", score2);
	TextOut(hDC, 100, 0, score1_str, 7);
	TextOut(hDC, 200, 0, score2_str, 7);

	if (score1 + score2 == 52) {
		TextOut(hDC, 300, 0, TEXT("終了"), 4);
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
					mark = "スペード";
				}
				else if (ban[(i * 13 + j)].number / 13 == 1) {
					mark = "ハート";
				}
				else if (ban[(i * 13 + j)].number / 13 == 2) {
					mark = "クローバー";
				}
				else if (ban[(i * 13 + j)].number / 13 == 3) {
					mark = "ダイヤ";
				}
				num = (ban[i * 13 + j].number % 13) + 1;
				sprintf_s(num_str, "%2d", num);
				TextOut(hDC, 20 + j * 60, 50 + i * 150, mark, lstrlen(mark));
				TextOut(hDC, 30 + j * 60, 90 + i * 150, num_str, lstrlen(num_str));
			}
		}
	}
	EndPaint(hWnd, &ps);            // 描画終了
	return 0L;
}
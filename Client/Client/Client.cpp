// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Client.h"
#include "afxsock.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			if (AfxSocketInit() == FALSE)
			{
				cout << "Khong the khoi tao";
				return FALSE;
			}
			// Tao socket
			CSocket client;
			client.Create();

			if (client.Connect(_T("127.0.0.1"), 8888) != 0) {
				cout << "Ket noi toi Server thanh cong !!!" << endl;
				string s_str;
				char r_str[256];
				int len;
				int rCode;
				do {
					cout << "Nhap nickname: ";
					fflush(stdin);
					getline(cin, s_str);
					client.Send(s_str.c_str(), s_str.length(), 0);

					client.Receive((char*)&rCode, sizeof(rCode), 0);

				} while ((rCode) == -1);
				if (rCode != -1) {
					cout << "Login Success" << endl << endl;
				}
				// Nhận thông báo trò chơi bắt đầu
				client.Receive((char*)&rCode, sizeof(rCode), 0);
				if (rCode == 2) {
					cout << "Game Start" << endl << endl;
				}

				// Bắt đầu trò chơi
				char col, disk;
				while (1) {
					char* rvStr = new char[256];
					int len;
					// Nhận đề
					len = client.Receive(rvStr, 256, 0);
					rvStr[len] = 0;
					cout << endl;
					cout << rvStr << endl;
					delete[]rvStr;
					// Gửi vị trí di chuyển
					cout << "Nhap dia can chuyen (ket thuc: 7): ";
					fflush(stdin);
					cin >> disk;
					cout << "Nhap cot chuyen dia den (ket thuc: 7): ";
					fflush(stdin);
					cin >> col;
					client.Send((char*)&disk, sizeof(disk), 0);
					client.Send((char*)&col, sizeof(col), 0);
					if (disk == '7' && col == '7') {
						break;
					}
					// Nhận kết quả
					client.Receive((char*)&rCode, sizeof(rCode), 0);

					while (rCode == -3) { // neu di chuyen khong hop le
						cout << "Di chuyen lai:" << endl;
						cout << "Nhap dia can chuyen: ";
						fflush(stdin);
						cin >> disk;
						cout << "Nhap cot chuyen dia den: ";
						fflush(stdin);
						cin >> col;
						client.Send((char*)&disk, sizeof(disk), 0);
						client.Send((char*)&col, sizeof(col), 0);
						if (disk == '7' && col == '7') {
							break;
						}
						// Nhận kết quả
						client.Receive((char*)&rCode, sizeof(rCode), 0);
					}
					if (disk == '7' && col == '7') {
						break;
					}
					if (rCode == 1) {
						break;
					}
				}
				if (disk == '7' && col == '7') {
					// Nhận số lần di chuyển
					int nMove = 0;
					client.Receive((char*)&nMove, sizeof(nMove), 0);
					cout << "So lan di chuyen: " << nMove << endl;
				}
				// Nhận kết quả dịch chuyển
				if (rCode == 1) {
					cout << "Ket qua: Hoan Thanh" << endl;
					// Nhận kết quả dịch chuyển
					client.Receive((char*)&rCode, sizeof(rCode), 0);
					cout << "So lan di chuyen: " << rCode << endl;
				}
				// Chờ người chơi khác
				client.Receive((char*)&rCode, sizeof(rCode), 0);
				if (rCode == 4) {
					// Nhận người chiến thắng
					len = client.Receive(r_str, 256, 0);
					r_str[len] = 0;
					cout << "Nguoi chien thang: " << r_str << endl;
				}
				system("pause");
			}
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

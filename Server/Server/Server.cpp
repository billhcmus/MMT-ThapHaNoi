// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"
#include "afxsock.h"
#include <vector>
#include <stack>
#include <string>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define BOCUOC -1
#define DANGCHOI 0
#define HOANTHANH 1
#define KETTHUCTROCHOI -2


// The one and only application object

CWinApp theApp;

using namespace std;

// Cấu trúc của player
struct PLAYER {
	SOCKET* skClient;
	stack<int> A, B, C;
	int stt = DANGCHOI;
	string nickName;
	int nMove = 0;
};

struct Info {
	string nickName;
	int nMove = 0;
};

// Mảng tạm thời chứa nickname của người chơi
vector<string> listNickName; // 3 người chơi
vector<Info> ketquachoi;
static int numPlayerRegister = 0; // người chơi đã đăng ký
int numPlayerConnected = 0;
int numPlayerCompleteOrGiveUp = 0;
// Phần khởi tạo đề
// Sắp xếp sao cho từ đáy lên đỉnh stack giảm dần
void Sort(stack<int> &st) {
	if (st.empty()) {
		return;
	}
	vector<int> vtTemp;
	while (!st.empty()) {
		vtTemp.push_back(st.top());
		st.pop();
	}
	sort(vtTemp.begin(), vtTemp.end());
	for (int i = vtTemp.size() - 1; i >= 0; i--) {
		st.push(vtTemp.at(i));
	}
}


void InitialCol(stack<int> &A, stack<int> &B, stack<int> &C) {
	srand(time(0));
	int numofdisk = rand() % 4 + 3;
	int *disk = new int[numofdisk]; // mang chua dia
	for (int i = 0; i < numofdisk; i++) {
		disk[i] = i + 1;
	}
	// cot A
	int nDiskInColA = rand() % (numofdisk + 1);
	for (int i = 0; i < nDiskInColA; i++) {
		int id;
		do {
			id = rand() % numofdisk;
			if (disk[id] != 0) {
				A.push(disk[id]);
				disk[id] = 0;
				break;
			}
		} while (disk[id] == 0);
	}
	Sort(A);
	// cot B
	int nDiskInColB = rand() % (numofdisk - nDiskInColA + 1);
	for (int i = 0; i < nDiskInColB; i++) {
		int id;
		do {
			id = rand() % numofdisk;
			if (disk[id] != 0) {
				B.push(disk[id]);
				disk[id] = 0;
				break;
			}
		} while (disk[id] == 0);
	}
	Sort(B);
	// cot C
	for (int i = 0; i < numofdisk; i++) {
		if (disk[i] != 0) {
			C.push(disk[i]);
		}
	}
	Sort(C);
}


char* getCol(stack<int> st) {
	char* s = new char[10];
	int i = 0;
	while (!st.empty()) {
		s[i++] = st.top() + 48;
		st.pop();
	}
	s[i] = 0;
	s = _strrev(s);
	return s;
}

string CreateAssignment(stack<int> a, stack<int> b, stack<int> c) {
	char* temp;
	string sA;
	string sB;
	string sC;
	string s_str;
	// cột A
	sA = "A: ";
	temp = getCol(a);
	sA += temp;
	// cột B
	sB = "B: ";
	temp = getCol(b);
	sB += temp;
	// cột C
	sC = "C: ";
	temp = getCol(c);
	sC += temp;

	s_str = sA + "\r\n" + sB + "\r\n" + sC;
	return s_str;
}

// kết thúc khởi tạo đề

bool checkNickName(string nickname) {
	if (nickname.length() > 10) {
		return false;
	}
	for (int i = 0; i < nickname.length(); i++) {
		if (!isalnum(nickname[i])) {
			return false;
		}
	}
	if (listNickName.empty()) {
		return true;
	}
	for (int i = 0; i < listNickName.size(); i++) {
		// trùng tên
		if (listNickName[i] == nickname) {
			return false;
		}
	}
	return true;
}

bool CheckMoveDisk(PLAYER* player, char col, char disk) {
	col = toupper(col);
	// Nếu nhập ngoài 3 cột ->false
	if (col != 'A' && col != 'B' && col != 'C') {
		return false;
	}
	int num = disk - 48;
	// nếu đĩa ngoài các đĩa trên cột
	if (num <= 0 || num > 6) {
		return false;
	}
	int topA = 0, topB = 0, topC = 0;
	if (!(player->A.empty())) {
		topA = player->A.top();
	}
	if (!(player->B.empty())) {
		topB = player->B.top();
	}
	if (!(player->C.empty())) {
		topC = player->C.top();
	}
	// nếu num lấy ra không ở đỉnh mỗi cột
	if (num != topA && num != topB && num != topC) {
		return false;
	}
	else { // num có ở top
		if (num == topA) { // nếu lấy num từ đỉnh A
			// Xét đỉnh bỏ vào
			if (col == 'B' && (topB > num || topB == 0)) {
				player->A.pop(); // bỏ ra khỏi A
				player->B.push(num); // bỏ vào B
				player->nMove++;
				return true;
			}
			else if (col == 'C' && (topC == 0 || topC > num)) {
				player->A.pop();
				player->C.push(num);
				player->nMove++;
				return true;
			}
			else {
				return false;
			}
		}
		else if (num == topB) {
			if (col == 'A' && (topA == 0 || topA > num)) {
				player->B.pop(); // bỏ ra khỏi B
				player->A.push(num); // bỏ vào A
				player->nMove++;
				return true;
			}
			else if (col == 'C' && (topC == 0 || topC > num)) {
				player->B.pop();
				player->C.push(num);
				player->nMove++;
				return true;
			}
			else {
				return false;
			}
		}
		else if (num == topC) {
			if (col == 'A' && (topA == 0 || topA > num)) {
				player->C.pop(); // bỏ ra khỏi C
				player->A.push(num); // bỏ vào A
				player->nMove++;
				return true;
			}
			else if (col == 'B' && (topB == 0 || topB > num)) {
				player->C.pop();
				player->B.push(num);
				player->nMove++;
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

bool CheckComplete(PLAYER* player) {
	if (player->A.empty() && player->B.empty()) {
		return true;
	}
	return false;
}

string FindWiner() {
	if (ketquachoi.size() == 0) { // Không có ai hoàn thành
		return "Khong co ai hoan thanh tro choi";
	}
	else if (ketquachoi.size() == 1) {
		return ketquachoi[0].nickName;
	}
	else if (ketquachoi.size() == 2) {
		if (ketquachoi[0].nMove < ketquachoi[1].nMove) {
			return ketquachoi[0].nickName;
		}
		else {
			return ketquachoi[1].nickName;
		}
	}
	int i;
	i = ketquachoi[0].nMove < ketquachoi[1].nMove ? 0 : 1;
	i = ketquachoi[i].nMove < ketquachoi[2].nMove ? i : 2;
	return ketquachoi[i].nickName;
}
// Tạo tiểu trình
DWORD WINAPI PlayGameThread(LPVOID pParam) {
	PLAYER* player = (PLAYER*)pParam;
	CSocket* ClientSocket = new CSocket();
	ClientSocket->Attach(*(player->skClient));
	if (!(ClientSocket == NULL || !ClientSocket->IsKindOf(RUNTIME_CLASS(CSocket)))) {
		string s_str; // chuỗi chứa dữ liệu send
		char r_str[256]; // chuỗi nhận dữ liệu
		char nickname[11];
		int len;
		int sCode;
		bool check;
		// Nhận nickname từ client
		len = ClientSocket->Receive(nickname, sizeof(nickname), 0);
		if (len == -1) {
			exit(0);
		}
		nickname[len] = 0;
		check = checkNickName(nickname);
		while (!check) {
			sCode = -1;
			ClientSocket->Send((char*)&sCode, sizeof(sCode), 0); // gửi mã lỗi
			len = ClientSocket->Receive(nickname, sizeof(nickname), 0);
			if (len == -1) {
				exit(0);
			}
			nickname[len] = 0;
			check = checkNickName(nickname);
		}
		// Nếu nickname hợp lệ
		sCode = 1;
		ClientSocket->Send((char*)&sCode, sizeof(sCode), 0); // gửi mã lỗi
		numPlayerRegister++;
		player->nickName = nickname;
		listNickName.push_back(player->nickName);
		player->stt = DANGCHOI;
		while (numPlayerRegister < 3) {
			Sleep(100);
		}
		cout << endl;
		if (numPlayerRegister == 3) {
			// thông báo trò chơi bắt đầu
			sCode = 2;
			ClientSocket->Send((char*)&sCode, sizeof(sCode), 0);
		}
		//--------------
		// Gửi đề bài cho player
		s_str = CreateAssignment(player->A, player->B, player->C);
		ClientSocket->Send(s_str.c_str(), s_str.length() + 1, 0);
		// Kết thúc gửi đề
		// Bắt đầu trò chơi
		while (player->stt == DANGCHOI) {
			// Nhận vị trí di chuyen của player
			char col, disk;
			len = ClientSocket->Receive((char*)&disk, sizeof(disk), 0);
			if (len == -1) {
				player->stt = BOCUOC;
				break;
			}
			len = ClientSocket->Receive((char*)&col, sizeof(col), 0);
			if (len == -1) {
				player->stt = BOCUOC;
				break;
			}
			if (col == '7' && disk == '7') {
				player->stt = KETTHUCTROCHOI;
				numPlayerCompleteOrGiveUp++;
				break;
			}
			check = CheckMoveDisk(player, col, disk);
			// Gửi kết quả
			if (check == false) { // di chuyen khong hop le
				sCode = -3;
				ClientSocket->Send((char*)&sCode, sizeof(sCode), 0);
			}
			else {	
				// Kiểm tra kết thúc
				if (CheckComplete(player)) {
					player->stt = HOANTHANH;
					numPlayerCompleteOrGiveUp++;
					Info temp;
					temp.nickName = player->nickName;
					temp.nMove = player->nMove;
					ketquachoi.push_back(temp);
					// xác nhận hoàn thành
					sCode = 1;
					ClientSocket->Send((char*)&sCode, sizeof(sCode), 0);
					// Gửi kết quả dịch chuyển
					ClientSocket->Send((char*)&player->nMove, sizeof(int), 0);
					break;
				}
				// Xác nhận đã chuyển đĩa
				sCode = 3;
				ClientSocket->Send((char*)&sCode, sizeof(sCode), 0);
				// thông báo kết quả dịch chuyển
				s_str = CreateAssignment(player->A, player->B, player->C);
				ClientSocket->Send(s_str.c_str(), s_str.length() + 1, 0);
			}
		}
		if (player->stt == KETTHUCTROCHOI) {
			// gửi số lần di chuyển
			ClientSocket->Send((char*)&player->nMove, sizeof(int), 0);
		}
		if (player->stt == HOANTHANH) {
			// Xác nhận hoàn thành
		}
		while (numPlayerCompleteOrGiveUp < 3) {
			Sleep(50);
		}
		// Thông báo các người chơi khác đã kết thúc
		sCode = 4;
		ClientSocket->Send((char*)&sCode, sizeof(sCode), 0);
		// Thông báo người chiến thắng
		s_str = FindWiner();
		ClientSocket->Send(s_str.c_str(), s_str.length(), 0);
	}
	return 0;
}



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
			if (AfxSocketInit() == FALSE) {
				cout << "Loi khoi tao";
				return FALSE;
			}
			CSocket server;
			AfxSocketInit();
			if (server.Create(8888, SOCK_STREAM, NULL) == FALSE) {
				cout << "Khoi tao that bai" << endl;
				server.GetLastError();
				return FALSE;
			}
			else {
				cout << "Khoi tao thanh cong" << endl;
				if (server.Listen(3) == FALSE) {
					cout << "khong the lang nghe tren cac port nay" << endl;
					server.Close();
					return FALSE;
				}
			}
				// Thread param
				DWORD dwPlayer_1_id, dwPlayer_2_id, dwPlayer_3_id;
				HANDLE hPlayer1Thread, hPlayer2Thread, hPlayer3Thread;
				// Khởi tạo đề
				stack<int> a, b, c; // các cột
				InitialCol(a, b, c);
				// Khởi tạo các player
				PLAYER player1, player2, player3;

				// Player1
				player1.A = a;
				player1.B = b;
				player1.C = c;

				CSocket* client1 = new CSocket();
				server.Accept(*client1);
				SOCKET hSocket1 = client1->Detach();
				player1.skClient = &hSocket1;

				numPlayerConnected++;
				hPlayer1Thread = CreateThread(NULL, 0, PlayGameThread, (LPVOID)&player1, 0, &dwPlayer_1_id);

				// Player2
				player2.A = a;
				player2.B = b;
				player2.C = c;

				CSocket* client2 = new CSocket();
				server.Accept(*client2);
				SOCKET hSocket2 = client2->Detach();
				player2.skClient = &hSocket2;

				numPlayerConnected++;
				hPlayer2Thread = CreateThread(NULL, 0, PlayGameThread, (LPVOID)&player2, 0, &dwPlayer_2_id);

				// Player3
				player3.A = a;
				player3.B = b;
				player3.C = c;

				CSocket* client3 = new CSocket();
				server.Accept(*client3);
				SOCKET hSocket3 = client3->Detach();
				player3.skClient = &hSocket3;
				numPlayerConnected++;
				hPlayer3Thread = CreateThread(NULL, 0, PlayGameThread, (LPVOID)&player3, 0, &dwPlayer_3_id);
				while (1);
				delete client1;
				delete client2;
				delete client3;
				TerminateThread(hPlayer1Thread, 0);
				TerminateThread(hPlayer1Thread, 0);
				TerminateThread(hPlayer1Thread, 0);
				ExitThread(0);

			
			return 0;
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

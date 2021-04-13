// orthelloGame.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include "pch.h"
#include <iostream>

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

#include <windows.h>
#include <wchar.h>
using namespace std;
//----------------------------------------------------
struct team
{
	int who;
	int A[8][8];

};
struct point
{
	int x;
	int y;
};
//-----------------------------------------------------
class filetool
{
public:
	static void checkupload()
	{
		system("dir .\\1082_IC192_A_ID_8 >list.txt");
		string tmp;
		ifstream file;
		file.open("list.txt");
		const int N = 1024;
		char aaa[N];
		int i;
		for (i = 0; i < 7; i++)
			file.getline(aaa, N);

		for (i = 0; i < 57; i++)
		{
			file.getline(aaa, N);
			tmp = aaa;
			size_t n = tmp.find("<DIR>");
			tmp = tmp.substr(n + 15);
			size_t m = tmp.find("_");
			if (tmp.compare(m + 1, tmp.length() - 1, "0") == 0) continue;
			//tmp = tmp.substr(0, m);
			cout << tmp << endl;
		}

		file.close();

	}
};
//-----------------------------------------------------
class Player
{
private:
	int order;
	string id;
	string name;
	HINSTANCE hDLL;
	struct point(*battle)(struct team t);
	static std::wstring charToWString(const char* text)
	{

		const size_t size = std::strlen(text);
		const size_t size2 = 64;
		size_t ReturnValue;
		wchar_t wcstr[64];
		wstring wstr;

		if (size > 0) {
			wstr.resize(size);
			mbstowcs_s(&ReturnValue, wcstr, size2, text, size);
			wstr = wcstr;
		}
		return wstr;
	}
public:
	Player()
	{
		hDLL = NULL;
		battle = NULL;
		order = 0;
	}
	~Player()
	{
		detatchDLL();
	}
	string getId()
	{
		return id;
	}
	wstring dllFileName()
	{
		wstring filename;
		filename = L"yzu";

		wstring wname = charToWString(id.c_str());
		filename.append(wname);
		filename.append(L".dll");
		return filename;
	}
	bool attachDLL()
	{
		wstring file;
		file = dllFileName();
		hDLL = LoadLibrary(file.c_str());

		if (hDLL)
		{
			(FARPROC&)battle = GetProcAddress(hDLL, "play");
			if (NULL == battle)
			{
				cout << "get fail" << endl;
				return false;
			}
			else
				return true;
		}
		else
		{
			cout << "load fail:" << id << endl;
			return false;
		}
	}
	bool detatchDLL()
	{
		if (hDLL)
		{
			FreeLibrary(hDLL);
			return true;
		}
		else
			return false;
	}
	bool hasDLL()
	{
		if (hDLL == NULL) return false;
		else return true;
	}
	//-------------------------------------
	struct point go(struct team t)
	{
		return  battle(t);

	}
	friend istream& operator>>(istream& is, Player& player)
	{
		is >> player.order;
		is >> player.id;
		is >> player.name;
		return is;
	}
	friend ostream& operator<<(ostream& os, Player& player)
	{
		os << player.order;
		os << player.id;
		os << player.name;
		return os;
	}
};
//-----------------------------------------------------
class RecordTable
{
private:
	vector<vector<int>> record;
public:
	void init(int amountPlayer)
	{
		record.resize(amountPlayer);
		int i;
		for (i = 0; i < amountPlayer; i++)
			record[i].resize(amountPlayer);
		reset();
	}
	void reset()
	{
		int i, j;
		size_t N = record.size();
		for (i = 0; i < N; i++)
			for (j = 0; j < N; j++)
				record[i][j] = 0;
	}
	void set(int m, int n, int status)
	{
		record[m][n] = status;
	}
	void setlose(int m, int n)
	{
		record[m][n]--;
	}
	int get(int m, int n)
	{
		return record[m][n];
	}
	//-----------------------------------------------------
	friend ostream& operator<<(ostream& os, RecordTable& recordtable)
	{
		int i, j;
		size_t N;
		N = recordtable.record.size();
		for (i = 0; i < N; i++)
		{
			for (j = 0; j < N; j++)
				os << setw(4) << recordtable.record[i][j];
			os << endl;
		}
		return os;
	}
};
//-----------------------------------------------------
struct threaddata
{
	struct team* t;
	struct point p;
	Player* player;
};
DWORD WINAPI thread(LPVOID data)
{
	cout << "ttt" << (((threaddata*)data)->t)->who << endl;
	(((struct threaddata*)data)->p) = ((struct threaddata*)data)->player->go(*(((threaddata*)data)->t));

	return 0;
}
class Game
{
private:
	int amountPlayer;
	vector<Player> list;
	RecordTable recordTable;

	struct team t;

	void init(struct team& t)
	{
		int i, j;
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++)
				t.A[i][j] = 0;

		t.A[3][3] = 1;
		t.A[4][4] = 1;
		t.A[3][4] = 2;
		t.A[4][3] = 2;

		t.who = 1;
	}
	void showTable(struct team& t)
	{
		int i, j;
		for (i = 0; i < 8; i++)
		{
			for (j = 0; j < 8; j++)
				cout << setw(2) << t.A[i][j];
			cout << endl;
		}
	}
	void setPlayPosition(struct point p)
	{
		t.A[p.x][p.y] = t.who;
		static struct point delta[] = { {1,0},{-1,0},{0,-1},{0,1},{-1,-1},{1,1},{-1,1},{1,-1} };
		int i;
		struct point current;
		for (i = 0; i < 8; i++)
		{
			current = p;
			current.x += delta[i].x;
			current.y += delta[i].y;
			t.A[current.x][current.y] = t.who;
			/*
			while ((t.A[current.x][current.y] != t.who) && current.x >= 0 && current.x < 8 && current.y >= 0 || current.y < 8 )
			{
				t.A[current.x][current.y] = t.who;
				current.x += delta[i].x;
				current.y += delta[i].y;
			}
			*/
		}
	}
	bool checkValid(struct point p)
	{
		return true;
		static struct point delta[] = { {1,0},{-1,0},{0,-1},{0,1},{-1,-1},{1,1},{-1,1},{1,-1} };
		bool ret = false;
		if (t.A[p.x][p.y] == 0)
		{
			for (int i = 0; i < 8; i++)
				if (t.A[p.x + delta[i].x][p.y + delta[i].y] != t.who) ret = true;
			return ret;
		}
		else
			return false;
	}
	bool checkwin(struct point p)
	{
		static int debug = 0;
		debug++;
		if (debug > 64) return true;

		int i, j;
		for (i = 0; i < 8; i++)
			for (j = 0; j < 8; j++)
				if (t.A[i][j] == 0) return false;


		return true;
	}
	bool playRound(int m, int n)
	{

		bool result = true;
		int pp[2];
		pp[0] = m;
		pp[1] = n;
		init(t);

		HANDLE hThread;
		struct threaddata d;
		d.t = &t;
		string teamname = list[pp[0]].getId();
		teamname.append("vs");
		teamname.append(list[pp[1]].getId());
		cout << teamname << endl;
		cout << "---------------------------" << endl;

		bool ret = true;

		while (result)
		{
			//----  以下請檢查邏輯 ----
			d.player = &(list[pp[t.who - 1]]);

			hThread = CreateThread(NULL, 0, thread, &d, 0, NULL);
			if (WAIT_TIMEOUT == WaitForSingleObject(hThread, 10000))
			{
				recordTable.setlose(pp[(t.who ^ 3) - 1], pp[(t.who) - 1]);
				ret = false;
				cout << "time out" << endl;
				break;
			}

			cout << "player" << d.player->getId() << ":" << d.p.x << "," << d.p.y << endl;
			if (checkValid(d.p))
			{
				setPlayPosition(d.p);
			}
			else
			{
				recordTable.setlose(pp[t.who - 1], pp[(t.who ^ 3) - 1]);
				ret = false;
				break;
			}
			if (checkwin(d.p))
			{
				int aa = (t.who ^ 3) - 1;
				int bb = (t.who) - 1;
				recordTable.setlose(pp[aa], pp[bb]);
				break;
			}

			t.who ^= 3;
			showTable(t);
			cout << endl;
			ofstream file;
			file.open(teamname + ".txt", std::ios_base::app);
			file << d.p.x << " " << d.p.y << endl;
			file.close();
		}
		return ret;
	}
public:
	Game()
	{
		amountPlayer = 0;
	}
	void loadGameTable(string filename)
	{
		ifstream file;
		file.open(filename);
		file >> amountPlayer;
		recordTable.init(amountPlayer);
		list.resize(amountPlayer);
		int i;
		for (i = 0; i < amountPlayer; i++)
			file >> list[i];
		file.close();
		//---------- debug -------
		for (i = 0; i < amountPlayer; i++)
			cout << list[i] << endl;
	}
	void playAll()
	{
		int i;
		int j;
		for (i = 0; i < amountPlayer; i++)
		{
			if (!list[i].attachDLL())
				recordTable.set(i, i, -1);
		}


		for (i = 0; i < amountPlayer; i++)
			for (j = 0; j < amountPlayer; j++)
				if (i != j && recordTable.get(i, i) != -1 && recordTable.get(j, j) != -1)
				{
					playRound(i, j);
				}


		cout << recordTable << endl;

		ofstream recordfile;
		TCHAR buf[64];
		wsprintf(buf, L"recordTable%d.csv", time(0));
		recordfile.open(buf);
		recordfile << recordTable;
		recordfile.close();

		for (i = 0; i < amountPlayer; i++)
			list[i].detatchDLL();

	}

};
class App
{

public:


	void run()
	{
		Game game;
		game.loadGameTable("playerlist.txt");
		game.playAll();
	}
};


int main()
{
	//filetool::checkupload();
	App app;
	app.run();

	return 1;
}


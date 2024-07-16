#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <limits>
#include <windows.h>

using namespace std;

void Clear() {
	system("cls");
}

//�����
#define red "\x1b[91m"              // �������
#define green "\x1b[92m"            // �������
#define light_blue "\x1b[94m"       // �����
#define reset "\x1b[0m"             // ����� �����

void CursoreVisable(bool a) {
	CONSOLE_CURSOR_INFO curs;
	curs.dwSize = 1;
	curs.bVisible = a;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curs);
}

// ���������
const int WIDTH = 10;
const int HEIGHT = 10;
const int MIN_MINES_COUNT = 5;

// ��������� ��� �������� ���������� � ������
struct Cell {
	bool isMine;       // �������� �� ������ �����
	bool isRevealed;   // ������� �� ������
	bool isFlagged;    // �������� �� ������ �������
	int adjacentMines; // ���������� ��� ������
};

class Minesweeper {
public:
	Minesweeper(int minesCount);
	void Play();

private:
	vector<vector<Cell>> field;
	int minesCount;
	bool gameOver;
	bool gameWon;
	chrono::time_point<chrono::steady_clock> startTime;

	void InitializeField();
	void PlaceMines();
	void CalculateAdjacentMines();
	void RevealCell(int x, int y);
	void ToggleFlag(int x, int y);
	void DisplayField();
	void DisplayEndMessage();
	bool IsGameWon();
	int CountRevealedCells();
	int CountAdjacentMines(int x, int y);
	void HandleMouseClick();
};

// ����������� �������������� ����
Minesweeper::Minesweeper(int minesCount) : minesCount(minesCount), gameOver(false), gameWon(false) {
	field.resize(HEIGHT, vector<Cell>(WIDTH));
	InitializeField();
}

// ������������� ����
void Minesweeper::InitializeField() {
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			field[y][x] = { false, false, false, 0 };
		}
	}
	PlaceMines();
	CalculateAdjacentMines();
	startTime = chrono::steady_clock::now();
}

// ����������� ���
void Minesweeper::PlaceMines() {
	srand(time(0));
	for (int i = 0; i < minesCount; ++i) {
		int x, y;
		do {
			x = rand() % WIDTH;
			y = rand() % HEIGHT;
		} while (field[y][x].isMine);
		field[y][x].isMine = true;
	}
}

// ������� ��� ������ ������ ������
void Minesweeper::CalculateAdjacentMines() {
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			if (!field[y][x].isMine) {
				field[y][x].adjacentMines = CountAdjacentMines(x, y);
			}
		}
	}
}

// ������� ���������� ��� ������ �������� ������
int Minesweeper::CountAdjacentMines(int x, int y) {
	int count = 0;
	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			int nx = x + dx, ny = y + dy;
			if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT && field[ny][nx].isMine) {
				count++;
			}
		}
	}
	return count;
}

// �������� ������
void Minesweeper::RevealCell(int x, int y) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || field[y][x].isRevealed || field[y][x].isFlagged) return;

	field[y][x].isRevealed = true;

	if (field[y][x].isMine) {
		gameOver = true;
	}
	else if (field[y][x].adjacentMines == 0) {
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				RevealCell(x + dx, y + dy);
			}
		}
	}
}

// ���������/������ ������
void Minesweeper::ToggleFlag(int x, int y) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || field[y][x].isRevealed) return;
	field[y][x].isFlagged = !field[y][x].isFlagged;
}

// ����������� �������� ����
void Minesweeper::DisplayField() {
	Clear();
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			if (field[y][x].isRevealed) {
				if (field[y][x].isMine) {
					cout << red << "* " << reset;
				}
				else {
					cout << field[y][x].adjacentMines << " ";
				}
			}
			else if (field[y][x].isFlagged) {
				cout << green << "F " << reset;
			}
			else {
				cout << light_blue << "# " << reset;
			}
		}
		cout << endl;
	}
}

// �������� �� ������
bool Minesweeper::IsGameWon() {
	int revealedCount = CountRevealedCells();
	return revealedCount + minesCount == WIDTH * HEIGHT;
}

// ������� ���������� �������� ������
int Minesweeper::CountRevealedCells() {
	int count = 0;
	for (const auto& row : field) {
		for (const auto& cell : row) {
			if (cell.isRevealed) count++;
		}
	}
	return count;
}

// ��������� � ���������� ����
void Minesweeper::DisplayEndMessage() {
	if (gameOver) {
		cout << "���� ��������! �� ���������." << endl;
	}
	else if (gameWon) {
		cout << "�����������! �� ��������." << endl;
	}
	auto endTime = chrono::steady_clock::now();
	auto gameDuration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
	cout << "����� ����: " << gameDuration << " ������." << endl;
}

// ��������� ������ ����
void Minesweeper::HandleMouseClick() {
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD prev_mode;
	GetConsoleMode(hInput, &prev_mode);
	SetConsoleMode(hInput, prev_mode | ENABLE_MOUSE_INPUT);

	while (!gameOver && !gameWon) {
		INPUT_RECORD inputRecord;
		DWORD events;
		ReadConsoleInput(hInput, &inputRecord, 1, &events);

		if (inputRecord.EventType == MOUSE_EVENT) {
			MOUSE_EVENT_RECORD mouseEvent = inputRecord.Event.MouseEvent;
			if (mouseEvent.dwEventFlags == 0) {
				int x = mouseEvent.dwMousePosition.X / 2; // ��������������, ��� ������ ������ ����� ������ 2 �������
				int y = mouseEvent.dwMousePosition.Y;

				if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
					RevealCell(x, y);
				}
				else if (mouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) {
					ToggleFlag(x, y);
				}

				gameWon = IsGameWon();
				DisplayField();
			}
		}
	}
}

// �������� ������� �������
void Minesweeper::Play() {
	DisplayField();
	HandleMouseClick();
	DisplayEndMessage();
}

int main() {
	setlocale(0, "");

	int minesCount;
	cout << "������� ���������� ��� (�� ����� " << MIN_MINES_COUNT << "): ";
	cin >> minesCount;
	if (minesCount < MIN_MINES_COUNT) {
		cout << "���������� ��� �� ����� ���� ����� " << MIN_MINES_COUNT << ". ���� ����� ���������." << endl;
		return 1;
	}

	Minesweeper game(minesCount);
	game.Play();
	return 0;
}

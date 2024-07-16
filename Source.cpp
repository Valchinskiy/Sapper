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

//Цвета
#define red "\x1b[91m"              // Красный
#define green "\x1b[92m"            // Зеленый
#define light_blue "\x1b[94m"       // Синий
#define reset "\x1b[0m"             // Сброс цвета

void CursoreVisable(bool a) {
	CONSOLE_CURSOR_INFO curs;
	curs.dwSize = 1;
	curs.bVisible = a;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curs);
}

// Константы
const int WIDTH = 10;
const int HEIGHT = 10;
const int MIN_MINES_COUNT = 5;

// Структура для хранения информации о клетке
struct Cell {
	bool isMine;       // Является ли клетка миной
	bool isRevealed;   // Открыта ли клетка
	bool isFlagged;    // Отмечена ли клетка флажком
	int adjacentMines; // Количество мин вокруг
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

// Конструктор инициализирует игру
Minesweeper::Minesweeper(int minesCount) : minesCount(minesCount), gameOver(false), gameWon(false) {
	field.resize(HEIGHT, vector<Cell>(WIDTH));
	InitializeField();
}

// Инициализация поля
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

// Расстановка мин
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

// Подсчет мин вокруг каждой клетки
void Minesweeper::CalculateAdjacentMines() {
	for (int y = 0; y < HEIGHT; ++y) {
		for (int x = 0; x < WIDTH; ++x) {
			if (!field[y][x].isMine) {
				field[y][x].adjacentMines = CountAdjacentMines(x, y);
			}
		}
	}
}

// Подсчет количества мин вокруг заданной клетки
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

// Открытие клетки
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

// Установка/снятие флажка
void Minesweeper::ToggleFlag(int x, int y) {
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT || field[y][x].isRevealed) return;
	field[y][x].isFlagged = !field[y][x].isFlagged;
}

// Отображение игрового поля
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

// Проверка на победу
bool Minesweeper::IsGameWon() {
	int revealedCount = CountRevealedCells();
	return revealedCount + minesCount == WIDTH * HEIGHT;
}

// Подсчет количества открытых клеток
int Minesweeper::CountRevealedCells() {
	int count = 0;
	for (const auto& row : field) {
		for (const auto& cell : row) {
			if (cell.isRevealed) count++;
		}
	}
	return count;
}

// Сообщение о завершении игры
void Minesweeper::DisplayEndMessage() {
	if (gameOver) {
		cout << "Игра окончена! Вы проиграли." << endl;
	}
	else if (gameWon) {
		cout << "Поздравляем! Вы выиграли." << endl;
	}
	auto endTime = chrono::steady_clock::now();
	auto gameDuration = chrono::duration_cast<chrono::seconds>(endTime - startTime).count();
	cout << "Время игры: " << gameDuration << " секунд." << endl;
}

// Обработка кликов мыши
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
				int x = mouseEvent.dwMousePosition.X / 2; // предполагается, что каждая ячейка имеет ширину 2 символа
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

// Основной игровой процесс
void Minesweeper::Play() {
	DisplayField();
	HandleMouseClick();
	DisplayEndMessage();
}

int main() {
	setlocale(0, "");

	int minesCount;
	cout << "Введите количество мин (не менее " << MIN_MINES_COUNT << "): ";
	cin >> minesCount;
	if (minesCount < MIN_MINES_COUNT) {
		cout << "Количество мин не может быть менее " << MIN_MINES_COUNT << ". Игра будет завершена." << endl;
		return 1;
	}

	Minesweeper game(minesCount);
	game.Play();
	return 0;
}

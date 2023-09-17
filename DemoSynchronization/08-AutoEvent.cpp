// Пример синхронизации потоков при помощи
// событий с автоматическим сбросом

#include <windows.h>
#include <iostream>

using namespace std;

volatile int n;
HANDLE hOutEvent, hAddEvent;

DWORD WINAPI thread(LPVOID)
{
	int i;
	
	for (i = 0; i < 10; i++)
	{
		++n;
		if (i == 4)
		{
			SetEvent(hOutEvent);
			WaitForSingleObject(hAddEvent, INFINITE);
		}
	}
		
	return 0;
}

int main()
{
	HANDLE	hThread;
	DWORD	IDThread;

	cout << "An initial value of n = " << n << endl;

	// создаем события с автоматическим сбросом
	hOutEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hOutEvent == NULL)
		return GetLastError();
	hAddEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hAddEvent == NULL)
		return GetLastError();

	// создаем поток счетчик thread
	hThread = CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
	if (hThread == NULL)
		return GetLastError();

	// ждем пока поток thread выполнит половину работы
	WaitForSingleObject(hOutEvent, INFINITE);
	// выводим значение переменной
	cout << "An intermediate value of n = " << n << endl;
	// разрешаем дальше работать потоку thread
	SetEvent(hAddEvent);

	WaitForSingleObject(hThread, INFINITE);

	cout << "A final value of n = " << n << endl;

	CloseHandle(hThread);

	CloseHandle(hOutEvent);
	CloseHandle(hAddEvent);

	return 0;
}
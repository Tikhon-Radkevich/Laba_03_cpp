// Пример работы синхронизированных потоков.
// Работает только в Windows 2000.

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <iostream>

using namespace std;

CRITICAL_SECTION cs;

DWORD WINAPI thread(LPVOID)
{
	int i,j;
	
	for (j = 0; j < 10; j++)
	{
		// попытка войти в критическую секцию
		TryEnterCriticalSection (&cs);
		for (i = 0; i < 10; i++)
		{
			cout << j << " ";
			cout.flush();
		}
		cout << endl;
		// выход из критической секции
		LeaveCriticalSection(&cs);
	}
		
	return 0;
}

int main()
{
	int i,j;
	HANDLE	hThread;
	DWORD	IDThread;
	// инициализируем критическую секцию
	InitializeCriticalSection(&cs);

	hThread=CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
	if (hThread == NULL)
		return GetLastError();

	// потоки синхронизированы, поэтому каждая
	// строка содержит только одинаковые числа
	for (j = 10; j < 20; j++)
	{
		// попытка войти в критическую секцию
		TryEnterCriticalSection(&cs);
		for (i = 0; i < 10; i++)
		{
			cout << j << " ";
			cout.flush();
		}
		cout << endl;
		// выход из критической секции
		LeaveCriticalSection(&cs);
	}
	// удаляем критическую секцию
	DeleteCriticalSection(&cs);
	// ждем завершения работы потока thread
	WaitForSingleObject(hThread, INFINITE);

	return 0;
}

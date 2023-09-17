// Пример работы синхронизированных потоков

#include <windows.h>
#include <iostream>

using namespace std;

CRITICAL_SECTION cs;

DWORD WINAPI thread(LPVOID)
{
	int i,j;
	
	for (j = 0; j < 10; j++)
	{
		// входим в критическую секцию
		EnterCriticalSection (&cs);
		for (i = 0; i < 10; i++)
		{
			cout << j << ' ';
			cout.flush();
		}
		cout << endl;
		// выходим из критической секции
		LeaveCriticalSection(&cs);
	}
		
	return 0;
}

int main()
{
	int i, j;
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
			// входим в критическую секцию
		EnterCriticalSection(&cs);
		for (i = 0; i < 10; i++)
		{
			cout << j << ' ';
			cout.flush();
		}
		cout << endl;
			// выходим из критической секции
		LeaveCriticalSection(&cs);
	}
	// закрываем критическую секцию
	DeleteCriticalSection(&cs);
	// ждем, пока поток thread закончит свою работу
	WaitForSingleObject(hThread, INFINITE);	

	return 0;
}
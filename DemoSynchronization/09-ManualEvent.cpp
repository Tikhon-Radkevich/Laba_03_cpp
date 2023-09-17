// Пример синхронизации потоков при помощи событий с ручным сбросом 

#include <windows.h>
#include <iostream>
using namespace std;

volatile int n,m;
HANDLE hOutEvent[2], hAddEvent;

DWORD WINAPI thread_1(LPVOID)
{
	int i;
	
	for (i = 0; i < 10; i++)
	{
		++n;
		if (i == 4)
		{
			SetEvent(hOutEvent[0]);
			WaitForSingleObject(hAddEvent, INFINITE);
		}
	}
		
	return 0;
}

DWORD CALLBACK thread_2(LPVOID)
{
	int i;
	
	for (i = 0; i < 10; i++)
	{
		++m;
		if (i == 4)
		{
			SetEvent(hOutEvent[1]);
			WaitForSingleObject(hAddEvent, INFINITE);
		}
	}
		
	return 0;
}

int main()
{
	HANDLE	hThread_1, hThread_2;
	DWORD	IDThread_1, IDThread_2;

	cout << "An initial values of n = " << n << ", m = " << m << endl;

	// создаем события с автоматическим сбросом
	hOutEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hOutEvent[0] == NULL)
		return GetLastError();
	hOutEvent[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hOutEvent[1] == NULL)
		return GetLastError();

	// создаем событие с ручным сбросом
	hAddEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hAddEvent == NULL)
		return GetLastError();

	// создаем потоки счетчики
	hThread_1 = CreateThread(NULL, 0, thread_1, NULL, 0, &IDThread_1);
	if (hThread_1 == NULL)
		return GetLastError();
	hThread_2 = CreateThread(NULL, 0, thread_2, NULL, 0, &IDThread_2);
	if (hThread_2 == NULL)
		return GetLastError();

	// ждем пока потоки счетчики выполнят половину работы
	WaitForMultipleObjects(2, hOutEvent, TRUE, INFINITE);
	cout << "An intermediate values of n = " << n
		 << ", m = " << m << endl;
	// разрешаем потокам счетчикам продолжать работу
	SetEvent(hAddEvent);
	// ждем завершения потоков
	WaitForSingleObject(hThread_1, INFINITE);
	WaitForSingleObject(hThread_2, INFINITE);

	cout << "A final values of n = " << n << ", m = " << m << endl;

	CloseHandle(hThread_1);
	CloseHandle(hThread_2);

	CloseHandle(hOutEvent[0]);
	CloseHandle(hOutEvent[1]);
	CloseHandle(hAddEvent);

	return 0;
}
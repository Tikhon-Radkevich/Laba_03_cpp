// Пример работы не синхронизированных потоков

#include <windows.h>
#include <iostream>

using namespace std;

DWORD WINAPI thread(LPVOID)
{
	int i,j;
	
	for (j = 0; j < 10; j++)
	{
		for (i = 0; i < 10; i++)
		{
			cout << j << ' ';
			cout << flush;
			Sleep(22);
		}
		cout << endl;
	}
		
	return 0;
}

int main()
{
	int i, j;
	HANDLE	hThread;
	DWORD	IDThread;

	hThread=CreateThread(NULL, 0, thread, NULL, 0, &IDThread);
	if (hThread == NULL)
		return GetLastError();

	// так как потоки не синхронизированы, 
	// то выводимые строки непредсказуемы
	for (j = 10; j < 20; j++)
	{
		for (i = 0; i < 10; i++)
		{
			cout << j << ' ';
			cout << flush;
			Sleep(22);
		}
		cout << endl;
	}
	// ждем, пока поток thread закончит свою работу
	WaitForSingleObject(hThread, INFINITE);

	return 0;
}
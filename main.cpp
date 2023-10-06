#include <windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <random>

using std::cout;
using std::cin;
using std::vector;

int* array;
int arraySize;

HANDLE* markerThreads; // Array of marker thread handles
HANDLE mainEvent; // Event for the main thread to wait on
HANDLE* markerEvents; // Array of marker threads to signal the main thread
HANDLE* StopMarkerEvents;

struct MarkerThreadArgs {
    int markerId;
};

// Marker thread
DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerThreadArgs* t_args = reinterpret_cast<MarkerThreadArgs*>(lpParam);
    int markerId = t_args->markerId;

    std::random_device rd;
    std::mt19937 rng(rd() + markerId);
    std::uniform_int_distribution<int> dist(0, arraySize-1);


    while (true) {
        // Wait for the main thread to signal start
        WaitForSingleObject(mainEvent, INFINITE);

        int indexToMark = dist(rng);

        Sleep(1000);
        if (array[indexToMark] == 0) {
            // Mark the element with the markerId
            array[indexToMark] = markerId;

            // for (int i = 0; i < arraySize; i++)
            // {
            //     cout << array[i] << " ";
            // }
            // cout << std::endl;

            // Sleep for another 5 milliseconds
            Sleep(5);


        } else {
            // Notify the main thread that marking is impossible
            SetEvent(markerEvents[markerId-1]);
            ResetEvent(StopMarkerEvents[markerId-1]);
            // cout << "\nSleep" << std::endl;
            WaitForSingleObject(StopMarkerEvents[markerId-1], INFINITE);
            // cout << "StopMarkerEvents\n";
        }
    }
    return 0;
}

int main() {
    cout << "Enter the size of the array: ";
    cin >> arraySize;

    // Allocate memory for the array
    array = new int[arraySize];
    memset(array, 0, sizeof(int) * arraySize);

    cout << "Enter the number of marker threads: ";
    int numMarkerThreads;
    cin >> numMarkerThreads;

    // Create an array of marker thread handles and events
    markerThreads = new HANDLE[numMarkerThreads];
    markerEvents = new HANDLE[numMarkerThreads];
    StopMarkerEvents = new HANDLE[numMarkerThreads];

    // Create the main event
    mainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Create marker threads
    for (int i = 0; i < numMarkerThreads; i++) {
        MarkerThreadArgs* thread_args = new MarkerThreadArgs{i+1};
        markerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        StopMarkerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        markerThreads[i] = CreateThread(NULL, 0, MarkerThread, thread_args, 0, NULL);
        // cout << "Marker thread " << i << " created\n";
    }

    // Signal marker threads to start
    SetEvent(mainEvent);

    int TerminatedThedsCount = 0;

    while (true) {
        // Wait for all marker threads to signal completion or impossibility
        DWORD waitResult = WaitForMultipleObjects(numMarkerThreads, markerEvents, FALSE, INFINITE);

        cout << "Wait res: " << int(waitResult) << "\n";
        Sleep(500);
        if (waitResult >= WAIT_OBJECT_0 && waitResult <= WAIT_OBJECT_0 + TerminatedThedsCount) {
            ResetEvent(mainEvent);

            for (int i=0; i < numMarkerThreads; i ++){
                SetEvent(StopMarkerEvents[i]);
                ResetEvent(markerEvents[i]);
            }

            // Print the array contents
            cout << "Array contents: ";
            for (int i = 0; i < arraySize; i++) {
                cout << array[i] << " ";
            }
            cout << std::endl;

            cout << "All marker threads completed\n";

            // Prompt for a marker thread to terminate
            int markerId;
            cout << "Enter the marker thread to terminate (1-" << numMarkerThreads << "): ";
            cin >> markerId;

            TerminateThread(markerThreads[markerId-1], 0);  

            TerminatedThedsCount += 1;  
            

            // Clear the marked elements
            for (int i = 0; i < arraySize; i++) {
                if (array[i] == markerId) {
                    array[i] = 0;
                }
            }
            
            SetEvent(mainEvent);
        }
    }

    // Clean up and release resources
    for (int i = 0; i < numMarkerThreads; i++) {
        CloseHandle(markerThreads[i]);
        CloseHandle(markerEvents[i]);
        CloseHandle(StopMarkerEvents[i]);
    }

    delete[] array;
    delete[] markerThreads;
    delete[] markerEvents;
    delete[] StopMarkerEvents;

    return 0;
}

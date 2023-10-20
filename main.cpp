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

HANDLE* markerThreads;
HANDLE mainEvent;
HANDLE* markerEvents;
HANDLE* StopMarkerEvents;
HANDLE mutex = CreateMutex(NULL, FALSE, NULL);

struct MarkerThreadArgs {
    int markerId;
    bool terminate = FALSE;
};

// Marker thread
DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerThreadArgs* t_args = reinterpret_cast<MarkerThreadArgs*>(lpParam);
    int markerId = t_args->markerId;

    std::random_device rd;
    std::mt19937 rng(rd() + markerId);
    std::uniform_int_distribution<int> dist(0, arraySize - 1);

    int counter = 0;
    while (true) {
        // Clear the marked elements && terminate thread
        if (t_args->terminate == TRUE) {
            for (int i = 0; i < arraySize; i++) {
                if (array[i] == markerId) {
                    array[i] = 0;
                }
            }
            return 0;
        }
        // Wait for the main thread to signal start
        WaitForSingleObject(mainEvent, INFINITE);

        int indexToMark = dist(rng);

        if (array[indexToMark] == 0) {
            // Mark the element with the markerId
            WaitForSingleObject(mutex, INFINITE);
            Sleep(5);
            array[indexToMark] = markerId;
            ReleaseMutex(mutex);
            counter++;
            Sleep(5);
        } else {
            // Notify the main thread that marking is impossible
            WaitForSingleObject(mutex, INFINITE);
            cout << "Marker: " << markerId << " ### Marked amount: " << counter << "\n";
            ReleaseMutex(mutex);
            ResetEvent(StopMarkerEvents[markerId - 1]);
            SetEvent(markerEvents[markerId - 1]);
            WaitForSingleObject(StopMarkerEvents[markerId - 1], INFINITE);
        }
    }
    return 0;
}

int main() {
    cout << "Enter the size of the array: ";
    cin >> arraySize;

    array = new int[arraySize];
    memset(array, 0, sizeof(int) * arraySize);

    cout << "Enter the number of marker threads: ";
    int numMarkerThreads;
    cin >> numMarkerThreads;

    // Create an array of marker thread handles and events
    markerThreads = new HANDLE[numMarkerThreads];
    markerEvents = new HANDLE[numMarkerThreads];
    StopMarkerEvents = new HANDLE[numMarkerThreads];
    MarkerThreadArgs* thread_args = new MarkerThreadArgs[numMarkerThreads];

    // Create the main event
    mainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Create marker threads
    for (int i = 0; i < numMarkerThreads; i++) {
        thread_args[i].markerId = i + 1;
        markerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        StopMarkerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        markerThreads[i] = CreateThread(NULL, 0, MarkerThread, &thread_args[i], 0, NULL);
    }

    // Signal marker threads to start
    SetEvent(mainEvent);

    int TerminatedThedsCount = 0;
    int markerId;
    bool thread_to_terminate_choice = TRUE;
    DWORD waitResult;

    for(int mark_iter=0; mark_iter < numMarkerThreads; mark_iter++) {
        SetEvent(mainEvent);

        // Wait for all marker threads to signal completion or impossibility
        waitResult = WaitForMultipleObjects(numMarkerThreads, markerEvents, TRUE, INFINITE);

        // cout << "\nwait res: " << waitResult << "\n";
        // DWORD waitResult_2 = WaitForMultipleObjects(numMarkerThreads, markerEvents, TRUE, INFINITE);
        // cout << "\nwait res_2: " << waitResult_2 << "\n";

        if (waitResult == WAIT_OBJECT_0) {
            ResetEvent(mainEvent);

            // Print the array contents
            cout << "\nArray contents: ";
            for (int i = 0; i < arraySize; i++) {
                cout << array[i] << " ";
            }

            while (thread_to_terminate_choice){
                // Prompt for a marker thread to terminate
                cout << "\n\nEnter the marker thread to terminate (1-" << numMarkerThreads << "): ";
                cin >> markerId;

                if (thread_args[markerId - 1].terminate == TRUE){
                    cout << "\nThread: " << markerId << " is terminated yeat. Choose another one.\n";
                    continue;
                } 
                else{    
                    thread_args[markerId - 1].terminate = TRUE;

                    for (int i = 0; i < numMarkerThreads; i++) {
                        SetEvent(StopMarkerEvents[i]);
                        if (thread_args[i].terminate == TRUE){
                            SetEvent(markerEvents[i]);
                        }
                    }
                    break;
                }
            }
            WaitForSingleObject(markerThreads[markerId - 1], INFINITE); // Wait for the thread to terminate
        }
    }

    cout << "All marker threads terminated. Exiting...\n";

    // Clean up and release resources
    for (int i = 0; i < numMarkerThreads; i++) {
        CloseHandle(markerThreads[i]);
        CloseHandle(markerEvents[i]);
        CloseHandle(StopMarkerEvents[i]);
    }
    CloseHandle(mutex);

    delete[] array;
    delete[] markerThreads;
    delete[] markerEvents;
    delete[] StopMarkerEvents;

    return 0;
}
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

struct MarkerThreadArgs {
    int markerId;
    bool terminate = FALSE;
    HANDLE mutex;
};

// Marker thread
DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerThreadArgs* t_args = reinterpret_cast<MarkerThreadArgs*>(lpParam);
    int markerId = t_args->markerId;

    WaitForSingleObject(t_args->mutex, INFINITE);
    cout << "Marker thread " << markerId << " started" << std::endl;
    ReleaseMutex(t_args->mutex);

    std::random_device rd;
    std::mt19937 rng(rd() + markerId);
    std::uniform_int_distribution<int> dist(0, arraySize - 1);

    int counter = 0;
    while (true) {
        if (t_args->terminate == TRUE) {
            // Clear the marked elements
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
            array[indexToMark] = markerId;
            counter++;
            Sleep(5);
        } else {
            // Notify the main thread that marking is impossible
            // WaitForSingleObject(t_args->mutex, INFINITE);
            // cout << "Marker: " << markerId << " ### Marked amount: " << counter << " ### Could not mark element: " << indexToMark << "\n";
            // ReleaseMutex(t_args->mutex);
            SetEvent(markerEvents[markerId - 1]);
            ResetEvent(StopMarkerEvents[markerId - 1]);
            WaitForSingleObject(StopMarkerEvents[markerId - 1], INFINITE);
            cout << "Marker: " << markerId << " ### Marked amount: " << counter << " ### Resuming marking\n";
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
    MarkerThreadArgs* thread_args = new MarkerThreadArgs[numMarkerThreads];

    // Create the main event
    mainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Create a mutex for cout protection
    HANDLE coutMutex = CreateMutex(NULL, FALSE, NULL);

    // Create marker threads
    for (int i = 0; i < numMarkerThreads; i++) {
        thread_args[i].markerId = i + 1;
        thread_args[i].mutex = coutMutex;
        markerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        StopMarkerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        markerThreads[i] = CreateThread(NULL, 0, MarkerThread, &thread_args[i], 0, NULL);
    }

    // Signal marker threads to start
    SetEvent(mainEvent);

    int TerminatedThedsCount = 0;

    while (true) {
        // Wait for all marker threads to signal completion or impossibility
        DWORD waitResult = WaitForMultipleObjects(numMarkerThreads, markerEvents, FALSE, INFINITE);

        if (waitResult >= WAIT_OBJECT_0 && waitResult <= WAIT_OBJECT_0 + TerminatedThedsCount) {
            ResetEvent(mainEvent);

            // Print the array contents
            WaitForSingleObject(coutMutex, INFINITE);
            cout << "Array contents: ";
            for (int i = 0; i < arraySize; i++) {
                cout << array[i] << " ";
            }
            cout << std::endl;
            ReleaseMutex(coutMutex);

            // Prompt for a marker thread to terminate
            int markerId;
            cout << "Enter the marker thread to terminate (1-" << numMarkerThreads << "): ";
            cin >> markerId;

            thread_args[markerId - 1].terminate = TRUE;

            for (int i = 0; i < numMarkerThreads; i++) {
                SetEvent(StopMarkerEvents[i]);
                ResetEvent(markerEvents[i]);
            }

            WaitForSingleObject(markerThreads[markerId - 1], INFINITE); // Wait for the thread to terminate

            TerminatedThedsCount += 1;

            if (TerminatedThedsCount == numMarkerThreads) {
                break;
            }

            SetEvent(mainEvent);
        }
    }

    cout << "All marker threads terminated. Exiting..." << std::endl;

    // Clean up and release resources
    for (int i = 0; i < numMarkerThreads; i++) {
        CloseHandle(markerThreads[i]);
        CloseHandle(markerEvents[i]);
        CloseHandle(StopMarkerEvents[i]);
    }

    // Close the cout mutex
    CloseHandle(coutMutex);

    delete[] array;
    delete[] markerThreads;
    delete[] markerEvents;
    delete[] StopMarkerEvents;

    return 0;
}
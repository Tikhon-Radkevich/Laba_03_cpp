#include <windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>


using std::cout;
using std::cin;
using std::vector;


int* array;
int arraySize;

HANDLE* markerThreads; // Array of marker thread handles
HANDLE mainEvent; // Event for main thread to wait on
HANDLE* markerEvents; // Array of marker threads to signal main thread

struct MarkerThreadArgs {
    int markerId;
};

// Marker thread
DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerThreadArgs args = *reinterpret_cast<MarkerThreadArgs*>(lpParam);
    int markerId = args.markerId;

    srand(static_cast<unsigned int>(time(NULL)) + markerId);

    

    while (true) {
        // Wait for the main thread to signal start
        WaitForSingleObject(mainEvent, INFINITE);

        cout << markerId << " MarkerThread started\n";

        // Generate a random number
        int randomNumber = rand();

        // Calculate the index to mark
        int indexToMark = randomNumber % arraySize;

        // Sleep for 5 milliseconds
        Sleep(5);

        // Check if the element at the calculated index is zero
        if (array[indexToMark] == 0) {
            // Mark the element with the markerId
            array[indexToMark] = markerId;

            // Sleep for another 5 milliseconds
            Sleep(5);

            // Signal the main thread that marking is complete
            SetEvent(markerEvents[markerId]);
        } else {
            // Notify the main thread that marking is impossible
            SetEvent(markerEvents[markerId]);
            WaitForSingleObject(mainEvent, INFINITE); // Wait for a response from the main thread
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

    // Create the main event
    mainEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Create marker threads
    for (int i = 0; i < numMarkerThreads; i++) {
        MarkerThreadArgs args;
        args.markerId = i;
        markerEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        markerThreads[i] = CreateThread(NULL, 0, MarkerThread, &args, 0, NULL);
    }

    // Signal marker threads to start
    SetEvent(mainEvent);

    while (true) {
        // Wait for all marker threads to signal completion or impossibility
        DWORD waitResult = WaitForMultipleObjects(numMarkerThreads, markerEvents, TRUE, INFINITE);

        if (waitResult == WAIT_OBJECT_0) {
            // All marker threads signaled completion

            // Print the array contents
            cout << "Array contents: ";
            for (int i = 0; i < arraySize; i++) {
                cout << array[i] << " ";
            }
            cout << std::endl;

            // Prompt for a marker thread to terminate
            int markerId;
            cout << "Enter the marker thread to terminate (0-" << numMarkerThreads - 1 << "): ";
            cin >> markerId;

            if (markerId >= 0 && markerId < numMarkerThreads) {
                // Signal the selected marker thread to terminate
                TerminateThread(markerThreads[markerId], 0);

                // Reset the marker thread event
                ResetEvent(markerEvents[markerId]);

                // Clear the marked elements
                for (int i = 0; i < arraySize; i++) {
                    if (array[i] == markerId) {
                        array[i] = 0;
                    }
                }
            }
        }
    }

    // Clean up and release resources
    for (int i = 0; i < numMarkerThreads; i++) {
        CloseHandle(markerThreads[i]);
        CloseHandle(markerEvents[i]);
    }

    delete[] array;
    delete[] markerThreads;
    delete[] markerEvents;

    return 0;
}

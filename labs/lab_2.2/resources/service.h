#ifndef SERVICE_H
#define SERVICE_H

#include <iostream>

#include <windows.h>
#include <errhandlingapi.h>    // GetLastError

#define BREAK_ON_ERROR(str) { dw_result = GetLastError(); std::cout << str << "error=" << dw_result << " (0x" << (ULONG_PTR)dw_result << ")" << std::endl; }
#define BREAK_WITH_ERROR(str, err) { dw_result = err; std::cout << str << ". error=" << dw_result << std::endl; }

/*
 * Start a service which has already been created.
 */
DWORD ServiceStart(char * cp_name);

/*
 * Stop a service.
 */
DWORD ServiceStop(char * cp_name);

/*
 * Create a new service.
 */
DWORD ServiceCreate(char * cp_name, char * cp_path);

/*
 * Destroy an existing service.
 */
DWORD ServiceDestroy(char * cp_name);

#endif

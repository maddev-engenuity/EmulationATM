#ifndef NAMEDPIPE_IMPERSONATION_H
#define NAMEDPIPE_IMPERSONATION_H

#include <windows.h>

/*
 * Creates a service to connect to our named pipe as the client.
 * Returns 0x0 if successful, returns last Windows error code otherwise.
 * Sources:
 *  - https://github.com/rapid7/meterpreter/blob/d338f702ce8cb7f4e550f005ececaf5f3cadd2bc/source/extensions/priv/server/elevate/namedpipe.c#L92
 */
DWORD CreateNPClient(const char * kPipeName);

/*
 * Creates a named pipe and a client thread that connects to the pipe using a
 * system service. After the client writes to the pipe, the server thread can
 * read from the pipe, preparing it for impersonation.
 * Returns NULL if failure encountered, otherwise returns handle to pipe.
 * Sources:
 *  - https://github.com/rapid7/meterpreter/blob/d338f702ce8cb7f4e550f005ececaf5f3cadd2bc/source/extensions/priv/server/elevate/namedpipe.c#L9
 *  - https://www.ired.team/offensive-security/privilege-escalation/t1134-access-token-manipulation
 */
HANDLE CreateNamedPipeAndConnectClient();

/*
 * Performs named pipe client impersonation, and elevates to SYSTEM.
 * Returns 0 if successful, -1 otherwise.
 */
bool ElevateToSystem(HANDLE h_pipe);

/*
 * Uses the thread token to open a new SYSTEM cmd.exe shell.
 * Returns 0 if successful, -1 otherwise.
 */
int CreateSystemShell();

#endif

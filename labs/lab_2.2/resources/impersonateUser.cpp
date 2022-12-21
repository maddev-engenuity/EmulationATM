#include "impersonateUser.h"

#include <iostream>

#include <windows.h>
#include <securitybaseapi.h>
#include <errhandlingapi.h>     // GetLastError

#include "helpers.h"

using namespace std;

/*
 * Logs on and impersonates user.
 * Returns false if logon or impersonation fails.
 */
bool ImpersonateUserWithCredentials(const char * kUsername, const char * kPassword,
                                    const char * kDomain) {
    /*
     ******************************* TODO: *******************************
     * 
     * Assessment Step 1:
     *    Using Windows API calls, logon and impersonate the domain admin 
     *    using the provided credentials.
     */
}

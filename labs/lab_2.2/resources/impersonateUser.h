#ifndef IMPERSONATE_USER_H
#define IMPERSONATE_USER_H

/*
 * Logs on and impersonates user.
 * Returns false if logon or impersonation fails.
 */
bool ImpersonateUserWithCredentials(const char * kUsername, const char * kPassword,
                                    const char * kDomain);

#endif

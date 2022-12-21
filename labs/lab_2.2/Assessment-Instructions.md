# Assessment Instructions

## :biohazard: Malware Warning

---

Fundamentally, this course entails executing publicly known adversary TTPs so that we can assess and improve cybersecurity.

As a result, many of our tools and resources will likely be flagged malicious by security products. We make every effort to ensure that our adversary emulation content is trusted and safe for the purpose of offensive security testing.

As a precaution, you should not perform these labs on any system that contains sensitive data. Additionally, you should never use capabilities and/or techniques taught in this course without first obtaining explicit written permission from the system/network owner(s).

## Accessing the Machines

Each Cyberranges machine should be accessed through the `console` option. When running the exploit, the Win10 and Win2019_DC should both be open in your browser. If required, credentials for the machines are as follows:

| Machine Name | Username | Password |
| ---------- | ---------- | ---------- |
| Kali | attacker | ATT&CK |
| Win10 | localUser | Passw0rd!@# |
| Win2019_DC | madAdmin | ATT&CK |

## Completing the Assessment

The Shamoon malware we are emulating will be written in C++ and should be developed on the Kali machine. The malware will move laterally from the Win10 machine
to the Win2019_DC machine, elevating its privileges along the way.
The completed exploit is broken down into the following steps, with the tasks that you need to complete marked with **TODO**:

1. **TODO**: <u>Programmatically Log on and Impersonate the madAdmin user </u> on the Win10 Machine.
2. Programatically copies `shamoon.exe` to the Win2019_DC machine.
3. The last step on the Win10 machine creates and runs a remote scheduled task as the madAdmin user on the Win2019_DC machine, which executes the malware in an elevated process.
4. **TODO**: Now that the malware is running in an elevated process, on the Win2019_DC machine, <u>use named pipe client impersonation to set the current process token to SYSTEM. </u> The malware uses a SYSTEM service to connect to the named pipe, allowing for named pipe impersonation.
5. **TODO**: After the current process token has been changed to a SYSTEM token, <u>open and duplicate the thread token as a primary token so that it can be used to spawn a new cmd.exe instance. </u> Once the `cmd.exe` shell is opened, you can verify that `nt authority\system` has been achieved by using the following command: `whoami`.

Visual Studio Code has been installed on the Kali Linux machine for you to use for development. Open the template files contained in `/home/attacker/Desktop/resources` in Visual Studio Code. This folder should contain the following files:

* helpers.cpp
* helpers.h
* impersonateUser.cpp
* impersonateUser.h
* main.cpp
* namedPipeImpersonation.cpp
* namedPipeImpersonation.h
* service.cpp
* service.h

Using what you have learned, complete steps 1, 4, and 5 from above in the functions within the following template files:

* `impersonateUser.cpp`
    * `bool ImpersonateUserWithCredentials()`
* `namedPipeImpersonation.cpp`
    * `bool ElevateToSystem()`
    * `int CreateSystemShell()`

The functions that should be edited are marked with `******** TODO: ********` and the associated step from above. **Do not modify or delete any existing code.**

These files should be filled out with the Windows API calls that you have learned about in the instructional videos. `impersonateUser.cpp` should have all of the calls related to user impersonation, while `namedPipeImpersonation.cpp` should contain all of the API calls necessary for elevating privileges to SYSTEM through named pipe client impersonation.

## Compiling Your Solution

Open up a terminal on the Kali linux machine, and navigate to the folder that contains your solution files (`/home/attacker/Desktop/resources`).
Using MinGW, compile your solution files into `shamoon.exe` with the following command:

`x86_64-w64-mingw32-g++-posix -Wall -Wextra -Werror -o shamoon.exe *.cpp -static`

This compiled binary should then be copied onto the Win10 machine. Move `shamoon.exe` to the SMB shared folder (`/home/attacker/Desktop/Share`) on the Kali machine.
Then, from the Win10 machine, copy `shamoon.exe` to the Desktop from the Linux shared folder (`C:\Users\localUser\Desktop\Linux Share`). You will have to click the `Linux Share` Desktop shortcut in order to move `shamoon.exe`.

## Running the Compiled Executable

After the executable is copied to the Win10 machine Desktop, open a **non-admin** powershell instance, and execute it with the following command:

`.\shamoon.exe madAdmin "ATT&CK" MAD.local targetDC`

If the exploit works successfully, you should see a `cmd.exe` shell open on the Win2019_DC machine. Occasionally powershell.exe and cmd.exe may hang, so you may need to click on the open shell and hit `enter` to continue.

## Troubleshooting and Potential Issues

If you encounter any issues while taking the assessment, please open a new GitHub issue describing the
encountered problem, and the remediation steps that you have taken.

### Issues with SMB Share
If you are unable to access the Kali shared folder from the Win10 machine, try these steps:

1. From a terminal on the Kali machine, enter the following command: `sudo service smbd restart`
2. Delete the `Linux Share` from the Win10 Desktop and recreate it:
    2a. Right click on the Desktop, and select New -> Shortcut
    2b. Type the network location, which should contain the IP address of the Kali machine: `\\192.168.125.110\Share`
    2c. Click `Next`, rename the shortcut (`Linux Share`), and then click `Finish`.

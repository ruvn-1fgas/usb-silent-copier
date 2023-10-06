# USB Silent Copier

USB Silent Copier is a Windows application that silently copies USB data.

## Installation

1. Clone the repository.
2. Run `build.bat`.
3. Create a task in the task scheduler (`Ctrl+R` - > `taskschd.msc`):
    - General:
        - Name: USB Silent Copier
        - Trigger: Event trigger:
            - Journal: Microsoft-Windows-DriverFrameworks-UserMode/Operational
            - Source: DriverFrameworks-UserMode
            - Event ID: 2003
        - Don't forget to enable journal logging (Microsoft-Windows-DriverFrameworks-UserMode/Operational), just `Ctrl+R` -> `eventvwr.msc` -> find the journal -> right click -> Properties -> Enable logging.
        - Action: Start a program:
            - Program/script: `path_to_exe\usb_copier.exe`.

You can skip the first two steps and just download the latest release.\
But don't do it, who knows what's in there 😉, better build it yourself.

## Usage

1. Plug in the USB.
2. Navigate to C:\CopiedDisks\.
3. You will see a log file and copied files in a folder with the current date.

### P.S.

If you want to exclude some USB devices from copying, you can add their SerialNumber to the `config.txt` file (one line per USB). You can find the SerialNumber in the Device Manager or in the log file.\
File should be in the same folder as the executable.

### By the way

This is for educational purposes only. I am not responsible for any damage caused by this program.
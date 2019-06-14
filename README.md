# RFIDscanner
Turns RFID's received by XBee (or any other serial device) into keystrokes

## Requirements

- Tested on Windows 10 with Python 3.6
- [pyserial](https://pypi.org/project/pyserial/)

## Usage

The usage case for this script is to quickly scan a large number of RFID ids into an Excel spreadsheet.

This script expects to receive a number string (hex codes 48 to 57) followed by a newline (hex 10). Upon receiving the newline, it will simulate a keypress of each number followed by a tab. Every RFID tag found is stored in memory, to avoid duplicates.

- Run the script in the background
- Open Excel and select the first cell
- Scan the RFID tags

- Press `ESC` to exit
- Press `r` to clear RFID memory (to rescan a tag)

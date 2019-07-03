from directkeys import PressKey
import serial.tools.list_ports
import msvcrt
import time

#
# Set the COM port here, or 
# set listcomports to True if you want to see a list of COM ports
#
comport = 'COM6'
listcomports = False
#
#

if listcomports:
    comlist = serial.tools.list_ports.comports()
    connected = []
    for element in comlist:
        connected.append(element.device)
    print("Connected COM ports: " + str(connected))
    exit()

ser = serial.Serial(port=comport, baudrate=57600)

print("Connected to: " + ser.portstr)
print("Press ESC to exit")

line = []
rfids = []

while True:

    # check for keypresses
    if msvcrt.kbhit():
        # escape key
        if ord(msvcrt.getch()) == 27:
            break
        # r key
        if ord(msvcrt.getch()) == 114:
            print("clearing rfid cache")
            rfids = []
    
    # check for serial data
    while ser.in_waiting:
        for c in ser.read():
            # if character is a number
            if c >= 48 and c <= 57:
                line.append(chr(c))
            # if character is newline
            if c == 10:
                if not line in rfids:
                    print("rfid: " + ''.join(line))

                    # press every character key
                    for char in line:
                        PressKey(ord(char))

                    # press tab key
                    PressKey(0x09)

                    rfids.append(line)
                line = []
                break

ser.close()

import serial
from crc_checksum import *
from time import sleep

commands = [
    "$setup,0,0,0",  # abort
    "$setup,0,0,1"
]

COM_PORT = 'COM9'
BAUD_RATE = 9600

prompt = serial.Serial(COM_PORT, BAUD_RATE, timeout=0,
                       parity=serial.PARITY_NONE)

while True:
    for msg in commands:
        prompt.write(signMessage(msg).encode())
        sleep(1)

from digitalio import DigitalInOut, Direction, Pull
import busio
import board
import adafruit_rfm9x
import time

# Based on https://learn.adafruit.com/lora-and-lorawan-radio-for-raspberry-pi/sending-data-using-a-lora-radio
# Documentation at https://docs.circuitpython.org/projects/rfm9x/en/latest/api.html#

CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, frequency = 915.5, preamble_length=8, baudrate=62500, crc=True)

rfm9x.coding_rate=5
rfm9x.tx_power = 5
rfm9x.spreading_factor = 7

counter = 0

while True:
    data = rfm9x.receive, keep_listening = True, with_header = True)

    print(f'Received {counter}: {data}')
    
    counter += 1
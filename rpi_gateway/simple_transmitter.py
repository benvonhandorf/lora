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
rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, frequency = 915.0, preamble_length=8, baudrate=125000, crc=True)

rfm9x.coding_rate=5
rfm9x.tx_power = 5
rfm9x.spreading_factor = 7

counter = 0

while True:
    data = bytes(f'Hello {counter}', 'utf-8')
    print(data)
    rfm9x.send(data)
    time.sleep(10)
    counter += 1
from digitalio import DigitalInOut, Direction, Pull
import busio
import board
import adafruit_rfm9x
import datetime
import struct
import socket

import os

from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# Based on https://learn.adafruit.com/lora-and-lorawan-radio-for-raspberry-pi/sending-data-using-a-lora-radio
# Documentation at https://docs.circuitpython.org/projects/rfm9x/en/latest/api.html#

CS = DigitalInOut(board.CE1)
RESET = DigitalInOut(board.D25)
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)

lora_node_id = 0x01

rfm9x = adafruit_rfm9x.RFM9x(spi, CS, RESET, frequency = 915.5, preamble_length=8, baudrate=125000, crc=True)

rfm9x.coding_rate=5
rfm9x.tx_power = 5
rfm9x.spreading_factor = 7

counter = 0

bucket = 'weather_data'
org = 'skyiron'

influx_host = os.environ['INFLUX_HOST']
influx_token = os.environ['INFLUX_TOKEN']

influx_client = InfluxDBClient(url=f'http://{influx_host}:8086', token=influx_token, org=org)
influx_writer = influx_client.write_api(write_options=SYNCHRONOUS)

while True:
    try:
        data = rfm9x.receive(keep_listening = True, with_header = True)

        if data is not None:
            lora_rssi = rfm9x.last_rssi

            lora_destination = data[0]
            lora_source = data[1]
            lora_message_id = data[2]
            lora_flags = data[3]

            print(f'{datetime.datetime.now().isoformat()} - Received {counter} @ {rfm9x.last_rssi}: {data}')

            if data[2] == 0x01:
                # Type 1 message.  Expect 6 floats
                float_data = struct.unpack_from('6f', data, 4)
                
                print(float_data)

                point = Point('reading').tag('unit', 'weather_station_1') \
                    .field('voltage', float_data[0]) \
                    .field('current', float_data[1]) \
                    .field('humidity', float_data[2]) \
                    .field('temperature', float_data[3]) \
                    .field('pressure', float_data[4]) \
                    .field('temperature2', float_data[5])

                influx_writer.write(bucket=bucket, record=point)

                lora_packet = Point('message').tag('source', lora_source) \
                    .tag('destination', lora_destination) \
                    .tag('receiver', lora_node_id) \
                    .tag('message_type', lora_message_id) \
                    .tag('flags', lora_flags) \
                    .field('rssi', lora_rssi)

                influx_writer.write(bucket='lora_signal_strength', record=lora_packet)
            else:
                print('Unrecognized data')
            
            counter += 1
    except Exception as e:
        print(f'Error: {type(e)}' )
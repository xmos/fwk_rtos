#!/usr/bin/env python3
# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import sys
import serial
import serial.tools.list_ports
import argparse

# USB VID:PID for test application
vid=0x20B1
pid=0x4000

max_read_size=4096

# example run
# python serial_send_receive.py -if0 tx_data0 -if1 tx_data1 -of0 rx_data0 -of1 rx_data1

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('-if0', help="Input file for port 0's transmit data.")
    parser.add_argument('-if1', help="Input file for port 1's transmit data.")
    parser.add_argument('-of0', help="Output file for port 0's received data.")
    parser.add_argument('-of1', help="Output file for port 1's received data.")

    parser.parse_args()
    args = parser.parse_args()

    return args

def main(if0, if1, of0, of1):

    all_ports = serial.tools.list_ports.comports()
    test_ports = []
    required_ports = 2

    for port in all_ports:
        if port.vid == vid and port.pid == pid:
            test_ports.append(port)

    if len(test_ports) != required_ports:
        print(f'Error: Expected {required_ports} serial ports, found { len(test_ports) }.')
        sys.exit(1)

    port0 = None
    port1 = None

    try:
        print(f'Opening port 0 ({test_ports[0].device}).')
        port0 = serial.Serial(test_ports[0].device)
        print(f'Opening port 1 ({test_ports[1].device}).')
        port1 = serial.Serial(test_ports[1].device)

        print('Writing CDC test data to port 0, reading from port 1.')
        with open(if0, 'rb') as in_file, open(of1, 'wb') as out_file:
            while 1:
                tx_data = in_file.read(max_read_size)

                if not tx_data:
                    break

                port0.write(tx_data)
                out_file.write(port1.read(len(tx_data)))

        print('Writing CDC test data to port 1, reading from port 0.')
        with open(if1, 'rb') as in_file, open(of0, 'wb') as out_file:
            while 1:
                tx_data = in_file.read(max_read_size)

                if not tx_data:
                    break

                port1.write(tx_data)
                out_file.write(port0.read(len(tx_data)))

    finally:
        if port0 is not None:
            print('Closing port 0.')
            port0.close()
        if port1 is not None:
            print('Closing port 1.')
            port1.close()


if __name__ == "__main__":
    args = parse_arguments()

    main(args.if0, args.if1, args.of0, args.of1)

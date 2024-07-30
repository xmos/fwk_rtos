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
required_ports = 2

N_PORTS_ERROR_MSG = "We need at least 2 COM ports to perform the tests"

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

def find_ports_by_vid_pid(rq=2, msg="COM ports missing"):
    all_ports = serial.tools.list_ports.comports()
    test_ports = [port for port in all_ports if port.vid == vid and port.pid == pid]
    assert (len(test_ports) >= rq), msg
    return test_ports

def transfer_data(input_file, output_file, write_port, read_port, max_read_size):
    with open(input_file, 'rb') as in_file, open(output_file, 'wb') as out_file:
        while True:
            tx_data = in_file.read(max_read_size)
            if not tx_data:
                break
            write_port.write(tx_data)
            out_file.write(read_port.read(len(tx_data)))

def main(if0, if1, of0, of1):
    
    test_ports = find_ports_by_vid_pid(required_ports, N_PORTS_ERROR_MSG)
    port0 = None
    port1 = None
    
    try:
        d0 = test_ports[0].device
        d1 = test_ports[1].device

        print(f'Opening port 0 ({d0}).')
        port0 = serial.Serial(d0)
        
        print(f'Opening port 1 ({d1}).')
        port1 = serial.Serial(d1)

        print('Writing CDC test data to port 0, reading from port 1.')
        transfer_data(if0, of1, port0, port1, max_read_size)

        print('Writing CDC test data to port 1, reading from port 0.')
        transfer_data(if1, of0, port1, port0, max_read_size)

    except Exception as e:
        print(e)
        sys.exit(1)
    
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

#!/usr/bin/env python3
# Copyright 2020-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import getpass
import struct
import argparse
import os

s = bytearray()

parser = argparse.ArgumentParser()

parser.add_argument(
    "--ssid",
    help="network SSID",
)
parser.add_argument(
    "--password",
    help="network password",
)
parser.add_argument(
    "--security",
    help="security",
)

args = parser.parse_args()

if not (args.ssid):
    if not (os.environ.get('LOCAL_WIFI_SSID')):
        ssid = input("Enter the WiFi network SSID: ")
        os.environ["LOCAL_WIFI_SSID"] = ssid
    else:
        ssid = os.environ['LOCAL_WIFI_SSID']
else:
    ssid = args.ssid

if not (args.password):
    if not (os.environ.get('LOCAL_WIFI_PASS')):
        password = getpass.getpass("Enter the WiFi network password: ")
        os.environ["LOCAL_WIFI_PASS"] = password
    else:
        password = os.environ['LOCAL_WIFI_PASS']
else:
    password = args.password

# WiFi security defaults to WPA. Acceptable values are 0=open, 1=WEP, 2=WPA
if not (args.security):
    security = "2"
else:
    security = eval(args.security)

bssid = b"\x00\x00\x00\x00\x00\x00"

s += struct.pack(
    "<32sxB6s32sxBxxi",
    bytearray(ssid, "ascii"),
    min(len(ssid), 32),
    bssid,
    bytearray(password, "ascii"),
    min(len(password), 32),
    eval(security),
)

file = open(f"{os.path.realpath(os.path.dirname(__file__))}/networks.dat", "wb")
file.write(s)
file.close()


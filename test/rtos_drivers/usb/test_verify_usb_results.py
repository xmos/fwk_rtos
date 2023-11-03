#!/usr/bin/env python
# Copyright 2021-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re

target_test_results_filename = "testing/target.rpt"
target_test_regex = r"^Tile\[(\d{1})\]\|FCore\[(\d{1})\]\|(\d+)\|TEST\|(\w{4}) USB$"

host_test_results_filename = "testing/host.rpt"
host_test_regex = r"^\[TEST PASS\]:"

def test_results():
    with open(target_test_results_filename, "r") as f:
        cnt = 0
        while 1:
            line = f.readline()

            if not line:
                assert cnt == 2 # each tile should report PASS
                break

            p = re.match(target_test_regex, line)

            if p:
                cnt += 1
                assert p.group(4).find("PASS") != -1

    with open(host_test_results_filename, "r") as f:
        cnt = 0
        while 1:
            line = f.readline()

            if not line:
                assert cnt == 1
                break

            p = re.match(host_test_regex, line)

            if p:
                cnt += 1

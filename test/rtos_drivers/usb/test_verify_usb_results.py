#!/usr/bin/env python
# Copyright 2021-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re
from pathlib import Path

top_level = Path(__file__).parents[3].absolute() # if changes, make sure this relative path is correct

target_test_results_filename = top_level / "testing/target.rpt"
target_test_regex = r"^Tile\[\d\]\|FCore\[\d\]\|\d+\|TEST\|PASS USB$"
target_test_required_count = 2

host_test_results_filename = top_level / "testing/host.rpt"
host_test_regex = r"^\[TEST PASS\]:"
host_test_required_count = 1


def validate_results(filename, regex, required_count):
    """Counts the number of matching eleements in a file given a regex

    Args:
        filename (str): path to the file
        regex (regex): regex expression
        required_count (int): number of elements expected
    """
    with open(filename, "r") as file:
        content = file.read()
    
    matches = re.findall(regex, content, re.MULTILINE)
    pass_count = len(matches)

    assert pass_count == required_count, f"Expected {required_count} passes, but found {pass_count}"


def test_results():
    validate_results(target_test_results_filename, target_test_regex, target_test_required_count)
    validate_results(host_test_results_filename, host_test_regex, host_test_required_count)


if __name__ == "__main__":
    test_results() # manual debug
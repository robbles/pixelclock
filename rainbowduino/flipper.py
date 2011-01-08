#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import re
import argparse
import sys

# Matches a line defining an ascii char
ascii_def = re.compile(r'{ ?(?:0x[0-9A-F]{1,2}, ?){7}0x[0-9A-F]{1,2} ?},?\s*//\s*(?P<char>[^ ])', re.IGNORECASE)
    
test_line = "    {0x0,0x38,0x44,0x4C,0x54,0x64,0x44,0x38},   // 0"
test_line2 = "    {0x0,0x20,0x20,0x7C,0x24,0x28,0x30,0x20},   // 4"

def flip_bits(hex_string):
    """ 
    Reverses the bits in a hex string
    """
    num = int(hex_string, 16)
    # Convert to 8-bit binary string
    binary = bin(num)[2:].rjust(8, '0')
    # Reverse the string
    flipped = ''.join(reversed(binary))
    # Convert back to hex string
    return hex(int(flipped, 2))

def replacer(flip_rows, flip_columns):
    """ 
    Returns a function that will flip rows, columns or both 
    """
    def replace_ascii(match):
        line = match.group(0)
        char = match.group('char')
        elements = re.findall(r'0x[0-9A-F]{1,2}', line, re.IGNORECASE)
        if flip_columns:
            elements = map(flip_bits, elements)
        if flip_rows:
            elements.reverse()
        return ''.join(['{', ','.join(elements), '},', ' // ', char])

    return replace_ascii


def main():
    parser = argparse.ArgumentParser(description='Flips rows or columns in ASCII char definitions')

    parser.add_argument('input', type=argparse.FileType('rU'), metavar='INPUT_FILE')
    parser.add_argument('output', type=argparse.FileType('w'), default=sys.stdout, nargs='?', metavar='OUTPUT_FILE')

    parser.add_argument('-r', '--rows', action='store_true', help='Flip rows of each char definition')
    parser.add_argument('-c', '--columns', action='store_true', help='Flip columns of each char definition')

    args = parser.parse_args()

    for line in args.input:
        flipped = ascii_def.sub(replacer(args.rows, args.columns), line)
        print >> args.output, flipped,

if __name__ == '__main__':
    main()


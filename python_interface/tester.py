#! /usr/bin/python3

import vata_interface

res = vata_interface.union('../tests/aut_timbuk_smaller/A0053', '../tests/aut_timbuk_smaller/A0054')
print(res.stdout)

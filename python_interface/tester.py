#! /usr/bin/python3

import vata_interface
import result_printer

res = vata_interface.inclusion('../tests/aut_timbuk_smaller/A0053', \
        '../tests/aut_timbuk_smaller/A0054')
result_printer.printResult(res)

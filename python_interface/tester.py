#! /usr/bin/python3

import vata_interface
import vata_result

res = [vata_interface.inclusion('../tests/aut_timbuk_smaller/A0053', \
        '../tests/aut_timbuk_smaller/A0054')]
with open("temp.txt",'w') as f:
    res[0].writeStderrToFile(f)
res.append(vata_interface.union('../tests/aut_timbuk_smaller/A0053', \
        '../tests/aut_timbuk_smaller/A0054'))
res.append(vata_interface.intersection('../tests/aut_timbuk_smaller/A0053', \
        '../tests/aut_timbuk_smaller/A0054'))
res.append(vata_interface.load('../tests/aut_timbuk_smaller/A0053'))
res.append(vata_interface.simulation('../tests/aut_timbuk_smaller/A0053'))

for r in res:
    r.print()

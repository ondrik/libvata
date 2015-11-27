import subprocess
from vata_result import VATAResult

VATA_PATH = '../build/cli/vata'
UTF8 = "utf-8"

def __decodeBytes(bts):
    return bts.decode(UTF8) if bts is not None else None

def runVata(command, timeout=None):
    """
        Function executes VATA CLI and returns it output printed to
        stdout and stderr

        @param command ... Specification of command to be run by CLI.
            It should be given as a list of string equalt to parameters
            that would be given to CLI directly in command line
        @return VATAResult saves output of CLI at stdout and stderr
    """
    vata = subprocess.Popen([VATA_PATH]+command, \
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    res = vata.communicate(timeout=timeout)
    returnCode = vata.returncode
    return VATAResult(__decodeBytes(res[0]), __decodeBytes(res[1]), returnCode)

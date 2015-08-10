import subprocess

VATA_PATH = '../build/cli/vata'
UTF8 = "utf-8"

class VATAResult:
    def __init__(self, stdout, stderr, result):
        self.stdout = stdout
        self.stderr = stderr
        self.result = result

def decodeBytes(bts):
    return bts.decode(UTF8) if bts is not None else None

def runVata(command, timeout=None):
    vata = subprocess.Popen([VATA_PATH]+command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    res = vata.communicate(timeout=timeout)
    returnCode = vata.returncode
    return VATAResult(decodeBytes(res[0]), decodeBytes(res[1]), returnCode)

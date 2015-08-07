class VATAResult:
    def __init__(self, stdin, stderr, result):
        self.stdin = stdin
        self.stderr = stderr
        self.result = result

def run_vata(command, timeout=None):
    vata = subprocess.Popen(command)
    res = vata.communicate(timeout=timeout)
    returnCode = vata.returncode
    return VATAResult(res[0], res[1], returnCode)

from vata_executor import VATAResult

def printIfNotNone(output):
    if output is not None:
        print(output)

def printResult(res):
    printIfNotNone(res.stdout)
    printIfNotNone(res.stderr)

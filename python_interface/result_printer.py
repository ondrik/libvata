from vata_executor import VATAResult

def __printIfNotNone(output):
    if output is not None:
        print(output)

def printResult(res):
    """
        Prints a result of VATA CLI execution that is stored
        in parameter res. Function prints stdout and stderr
        output of the VATA CLI if there is such an output.

        @param res ... Result of VATA CLI output that is supposed
            to be clas VATAResult from module vata_executor
    """
    __printIfNotNone(res.stdout)
    __printIfNotNone(res.stderr)

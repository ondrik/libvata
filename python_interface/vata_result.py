class VATAResult(object):
    """
        Class describing stdout and stderr output
        of VATA CLI exection.
    """
    def __init__(self, stdout, stderr, result):
        self.stdout = stdout
        self.stderr = stderr
        self.result = result

    def __printIfNotNone(self, output):
        if output is not None:
            print(output)
            
    def print(self):
        """
            Prints a result of VATA CLI execution that is stored
            in this class data members. Function prints stdout and stderr
            output of the VATA CLI if there is such an output.
        """
        self.__printIfNotNone(self.stdout)
        self.__printIfNotNone(self.stderr)

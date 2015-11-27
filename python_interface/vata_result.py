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
    
    def __writeIfNotNone(self, outputString, outputFile):
        if outputString is not None:
            outputFile.write(outputString)
            
    def print(self):
        """
            Prints a result of VATA CLI execution that is stored
            in this class data members. Function prints stdout and stderr
            output of the VATA CLI if there is such an output.
        """
        self.__printIfNotNone(self.stdout)
        self.__printIfNotNone(self.stderr)

    def printStdout(self):
        self.__printIfNotNone(self.stdout)

    def printStderr(self):
        self.__printIfNotNone(self.stderr)

    def writeToFile(self, outputFile):
        self.__writeIfNotNone(self.stdout, outputFile)
        self.__writeIfNotNone(self.stderr, outputFile)

    def writeStdoutToFile(self, outputFile):
        self.__writeIfNotNone(self.stdout, outputFile)

    def writeStderrToFile(self, outputFile):
        self.__writeIfNotNone(self.stderr, outputFile)

    def writeToPath(self, outputPath):
        with open(outputPath, 'w') as f:
            self.writeToFile(f)

    def writeStdoutToPath(self, outputPath):
        with open(outputPath, 'w') as f:
            self.writeStdoutToFile(f)

    def writeStderrToPath(self, outputPath):
        with open(outputPath, 'w') as f:
            self.writeStderrToFile(f)

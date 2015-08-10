import cli_options_enums

class Command(object):
    """
        Class represents command of VATA CLI. It keeps information
        about operation, operands and additional options if needed.
        Operands are represented by a list of the path to files
        over which an operation should be performed.
        Operation is one from the enumeration given in cli_options_enums
        and the additional opetions can be specified by structures
        in module operation_options.
    """
    def __init__(self, encoding=cli_options_enums.EncodingsEnum.EXPL, \
            instructionCode=cli_options_enums.Unknown.UNKNOWN, \
            operands=[], options=None):
        self.__enconding = encoding
        self.__operation = instructionCode
        self.__operands = operands
        self.__options = options

    def getEncoding(self):
        return self.__enconding

    def getOperation(self):
        return self.__operation

    def getOperands(self):
        return self.__operands

    def getOptions(self):
        return self.__options

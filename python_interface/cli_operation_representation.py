import cli_options_enums
from operation_options import *

class Command:
    """
        Class represents command of VATA CLI. It keeps information
        about operation, operands and additional options if needed.
        Operands are represented by a list of the path to files
        over which an operation should be performed.
        Operation is one from the enumeration given in cli_options_enums
        and the additional opetions can be specified by structures
        in module operation_options.
    """
    def __init__(self):
        self.__enconding = cli_options_enums.EncodingsEnum.EXPL
        self.__operation = cli_options_enums.Unknown.UNKNOWN
        self.__operands = []
        self.__options = None

    def __init__(self, encoding, instructionCode, operands, options=None):
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

    def __loadOptionStructure(self, instructionCode):
        if instructionCode == cli_options_enums.OperationsEnum.INCL:
            return InclusionOption()
        elif instructionCode == cli_options_enums.OperationsEnum.SIM:
            return DirectionOption()
        elif instructionCode == cli_options_enums.OperationsEnum.EQUIV:
            return EquivOption()
        elif instructionCode == cli_options_enums.OperationsEnum.RED:
            return DirectionOption()
        else:
            return None

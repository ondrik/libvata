import cli_options_enums
from operation_options import *

class Command:
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
        if instructionCode == cli_options_enums.InstructionsEnum.INCL:
            return InclusionOption()
        elif instructionCode == cli_options_enums.InstructionsEnum.SIM:
            return DirectionOption()
        elif instructionCode == cli_options_enums.InstructionsEnum.EQUIV:
            return EquivOption()
        elif instructionCode == cli_options_enums.InstructionsEnum.RED:
            return DirectionOption()
        else:
            return None

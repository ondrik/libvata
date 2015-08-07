import cli_options_enums

class InclusionOption:
    def __init__(self, \
            direction = cli_options_enums.DirectionsEnum.DOWN, \
            alg = cli_options_enums.AlgsEnum.AC, \
            sim = False, \
            order = cli_options_enums.OrderEnum.DEPTH, \
            cacheOptimization = False, \
            recursion = False, \
            simulationTime = True):
        if not cli_options_enums.isCorrectCode(direction, cli_options_enums.DirectionsEnum()):
            raise Exception("Unknown direction code")
        if not cli_options_enums.isCorrectCode(alg, cli_options_enums.AlgsEnum()):
            raise Exception("Unknown algorithm code")
        if not cli_options_enums.isCorrectCode(alg, cli_options_enums.OrderEnum()):
            raise Exception("Unknown order code")

        self.__direction = direction
        self.__alg = alg
        self.__sim = sim
        self.__order = order
        self.__cacheOptimization = cacheOptimization
        self.__recursion = recursion
        self.__simulationTime = simulationTime

class DirectionOption:
    def __init__(self, direction = cli_options_enums.DirectionsEnum.DOWN):
        if not cli_options_enums.isCorrectCode(direction, cli_options_enums.DirectionsEnum()):
            raise Exception("Unknown direction code")

        self.__dir = direction

class EquivOption:
    def __init__(self, order = cli_options_enums.OrderEnum.DEPTH):
        if not cli_options_enums.isCorrectCode(order, cli_options_enums.OrderEnum()):
            raise Exception("Unknown order code")

        self.__order = order

class Operation:
    def __init__(self):
        self.__operation = cli_options_enums.Unknown.UNKNOWN
        self.__options = None
        self.__operands = []

    def __init__(self, instructionCode, options, operands):
        self.__operation = instructionCode
        self.__options = options
        self.__operands = operands

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

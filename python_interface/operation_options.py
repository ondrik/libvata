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
    
    def getDirection():
        return self.__dir
    
    def getOrder():
        return self.__order

class DirectionOption:
    def __init__(self, direction = cli_options_enums.DirectionsEnum.DOWN):
        if not cli_options_enums.isCorrectCode(direction, cli_options_enums.DirectionsEnum()):
            raise Exception("Unknown direction code")

        self.__dir = direction

    def getDirection():
        return self.__dir

class EquivOption:
    def __init__(self, order = cli_options_enums.OrderEnum.DEPTH):
        if not cli_options_enums.isCorrectCode(order, cli_options_enums.OrderEnum()):
            raise Exception("Unknown order code")

        self.__order = order
    
    def getOrder():
        return self.__order

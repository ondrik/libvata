import cli_options_enums

# This module provides classes representing options
# for operations like inclusion, simulation etc. where
# additional properties of the operation can be specified
# in VATA.

DIR_PARAM = 'dir='
ORDER_PARAM = 'order='
ALG_PARAM = 'alg='
SIM_PARAM = 'sim='
OPTC_PARAM = 'optC='
REC_PARAM = 'rec='
TIMES_PARAM = 'timeS='

class InclusionOption(object):
    def __init__(self, \
            direction=cli_options_enums.DirectionsEnum.DOWN, \
            alg=cli_options_enums.AlgsEnum.AC, \
            sim=False, \
            order=cli_options_enums.OrderEnum.DEPTH, \
            cacheOptimization=False, \
            recursion=False, \
            simulationTime=True):
        if not cli_options_enums.isCorrectCode(direction, \
                cli_options_enums.DirectionsEnum()):
            raise Exception("Unknown direction code")
        if not cli_options_enums.isCorrectCode( \
                alg, cli_options_enums.AlgsEnum()):
            raise Exception("Unknown algorithm code")
        if not cli_options_enums.isCorrectCode( \
                alg, cli_options_enums.OrderEnum()):
            raise Exception("Unknown order code")

        self.__direction = direction
        self.__alg = alg
        self.__sim = sim
        self.__order = order
        self.__cacheOptimization = cacheOptimization
        self.__recursion = recursion
        self.__simulationTime = simulationTime

    def getDirection(self):
        return self.__direction

    def getOrder(self):
        return self.__order

    def getAlg(self):
        return self.__alg

    def getSim(self):
        return self.__sim

    def getCacheOptimization(self):
        return self.__cacheOptimization

    def getRecursion(self):
        return self.__recursion

    def getSimulationTime(self):
        return self.__simulationTime

    def serialize(self):
        return DIR_PARAM + cli_options_enums.DirectionsEnumToString[ \
                self.getOrder()] + ',' + \
            ORDER_PARAM + cli_options_enums.OrderEnumToString[ \
                self.getOrder()] + ',' + \
            ALG_PARAM + cli_options_enums.AlgsEnumToString[ \
                self.getAlg()] + ','+ \
            SIM_PARAM + cli_options_enums.BoolToString[self.getSim()] + ','+ \
            OPTC_PARAM + cli_options_enums.BoolToString[ \
                self.getCacheOptimization()] + ',' + \
            REC_PARAM + cli_options_enums.BoolToString[self.getRecursion()] \
            + ',' + \
            TIMES_PARAM + cli_options_enums.BoolToString[ \
                self.getSimulationTime()]


class DirectionOption(object):
    def __init__(self, direction=cli_options_enums.DirectionsEnum.DOWN):
        if not cli_options_enums.isCorrectCode( \
                direction, cli_options_enums.DirectionsEnum()):
            raise Exception("Unknown direction code")

        self.__dir = direction

    def getDirection(self):
        return self.__dir

    def serialize(self, enc=cli_options_enums.EncodingsEnum.EXPL):
        if enc == cli_options_enums.EncodingsEnum.EXPL_FA:
            return DIR_PARAM + cli_options_enums.DirectionsEnumFAToString[ \
                    self.getDirection()]
        else:
            return DIR_PARAM + cli_options_enums.DirectionsEnumToString[ \
                    self.getDirection()]



class EquivOption(object):
    def __init__(self, order=cli_options_enums.OrderEnum.DEPTH):
        if not cli_options_enums.isCorrectCode( \
                order, cli_options_enums.OrderEnum()):
            raise Exception("Unknown order code")

        self.__order = order

    def getOrder(self):
        return self.__order

    def serialize(self):
        return ORDER_PARAM + \
                cli_options_enums.OrderEnumToString[self.getOrder()]

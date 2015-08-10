class Unknown:
    UNKNOWN = -1

class InstructionsEnum:
    LOAD = 0
    WITNESS = 1
    CMPL = 2
    UNION = 3
    ISECT = 4
    SIM = 5
    RED = 6
    EQUIV = 7
    INCL = 8
    HELP = 9

    MIN = LOAD
    MAX = HELP

InstructionsToStringCommand = {\
       InstructionsEnum.LOAD : 'load',
       InstructionsEnum.WITNESS : 'witness',
       InstructionsEnum.CMPL : 'cmpl',
       InstructionsEnum.UNION : 'union',
       InstructionsEnum.ISECT : 'isect',
       InstructionsEnum.SIM : 'sim',
       InstructionsEnum.RED : 'red',
       InstructionsEnum.EQUIV : 'equiv',
       InstructionsEnum.INCL : 'incl',
       InstructionsEnum.HELP : 'help'}

class DirectionsEnum:
    DOWN = 0
    UP = 1
    FWD = 0
    BWD = 1

    MIN = DOWN
    MAX = UP

DirectionsEnumFAToString = {\
        DirectionsEnum.BWD : 'bwd',
        DirectionsEnum.FWD : 'fwd'}

DirectionsEnumToString = {\
        DirectionsEnum.DOWN : 'down',
        DirectionsEnum.UP : 'up'}


class OrderEnum:
    BREADTH = 0
    DEPTH = 1
    
    MIN = BREADTH
    MAX = DEPTH

OrderEnumToString = {\
        OrderEnum.BREADTH : 'breadth',
        OrderEnum.DEPTH : 'depth'}


class AlgsEnum:
    AC = 0
    CONGR = 1
    
    MIN = AC
    MAX = CONGR

AlgsEnumToString = {\
        AlgsEnum.AC : 'antichains',
        AlgsEnum.CONGR : 'congr'}

BoolToString = {\
        True : 'yes',
        False : 'no'}

class EncodingsEnum:
    EXPL = 0
    BDD_TD = 1
    BDD_BU = 2
    EXPL_FA = 3
    
    MIN = EXPL
    MAX = EXPL_FA

EncodingToString = {\
        EncodingsEnum.EXPL : 'expl',
        EncodingsEnum.EXPL_FA : 'expl_fa',
        EncodingsEnum.BDD_TD : 'bdd-td',
        EncodingsEnum.BDD_BU : 'bdd-bu'}

def isCorrectCode(code, enum):
    if code >= enum.MIN and instruction <= enum.MAX:
        return True
    else:
        return False

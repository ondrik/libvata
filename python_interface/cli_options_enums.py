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

    MIN = LOAD
    MAX = INCL

class DirectionsEnum:
    DOWN = 0
    UP = 1
    FWD = 0
    BWD = 1

    MIN = DOWN
    MAX = UP

class OrderEnum:
    BREADTH = 0
    DEPTH = 1
    
    MIN = BREADTH
    MAX = DEPTH

class AlgsEnum:
    AC = 0
    CONGR = 1
    
    MIN = AC
    MAX = CONGR

class EncodingsEnum:
    EXPL = 0
    BDD-TD = 1
    BDD-BU = 2
    EXPL-FA = 3
    
    MIN = EXPL
    MAX = EXPL-FA

def isCorrectCode(code, enum):
    if code >= enum.MIN and instruction <= enum.MAX:
        return True
    else:
        return False

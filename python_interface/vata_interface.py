from cli_options_enums import OperationsEnum, EncodingsEnum
from cli_operation_representation import Command
import vata_executor
import command_serializer

# This module provides functions corresponding to the
# operations that VATA provides over tree automata
# Operands (parameters aut, lhs, rhs) should be path to the files
# where are the automata stored.
# It is possible also to choose encoding by parameter enc
# in which automata will be represented.
# The encoding should be represented by class EncodingsEnum
# from module cli_options_enums.
# It is also possible to specify other options that
# are provided by VATA CLI by parameter options (if it is available).
# The options should be represented by the class from
# module operation_options

def load(aut, enc=EncodingsEnum.EXPL):
    return __runCommand(Command(enc, OperationsEnum.LOAD, [aut]))

def complement(aut, enc=EncodingsEnum.EXPL):
    return __runCommand(Command(enc, OperationsEnum.CMPL, [aut]))

def witness(aut, enc=EncodingsEnum.EXPL):
    return __runCommand(Command(enc, OperationsEnum.WITNESS, [aut]))

def intersection(lhs, rhs, enc=EncodingsEnum.EXPL):
    return __runCommand(Command(enc, OperationsEnum.ISECT, [lhs, rhs]))

def union(lhs, rhs, enc=EncodingsEnum.EXPL):
    return __runCommand(Command(enc, OperationsEnum.UNION, [lhs, rhs]))

def simulation(aut, enc=EncodingsEnum.EXPL, options=None):
    return __runCommand(Command(enc, OperationsEnum.SIM, [aut], options))

def reduction(aut, enc=EncodingsEnum.EXPL, options=None):
    return __runCommand(Command(enc, OperationsEnum.RED, [aut], options))

def inclusion(lhs, rhs, enc=EncodingsEnum.EXPL, options=None):
    return __runCommand(Command(enc, OperationsEnum.INCL, [lhs, rhs], options))

def equivalence(lhs, rhs, enc=EncodingsEnum.EXPL, options=None):
    return __runCommand(Command(enc, OperationsEnum.EQUIV, [lhs, rhs], options))

def __runCommand(operation):
    return vata_executor.runVata(command_serializer.serializeCommand(operation))

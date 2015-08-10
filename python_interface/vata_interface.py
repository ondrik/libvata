from cli_options_enums import InstructionsEnum, EncodingsEnum
from cli_operation_representation import Operation
import vata_executor
import command_serializer

def load(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Operation(enc, InstructionsEnum.LOAD, [aut]))

def complement(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Operation(enc, InstructionsEnum.CMPL, [aut]))

def witness(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Operation(enc, InstructionsEnum.WITNESS, [aut]))

def intersection(lhs,rhs, enc=EncodingsEnum.EXPL):
    return runCommand(Operation(enc, InstructionsEnum.ISECT, [lhs,rhs]))

def union(lhs, rhs, enc=EncodingsEnum.EXPL):
    return runCommand(Operation(enc, InstructionsEnum.UNION, [lhs,rhs]))

def simulation(aut, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Operation(aut, InstructionsEnum.SIM, [aut], options))

def reduction(aut, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Operation(aut, InstructionsEnum.RED, [aut], options))

def inclusion(lhs,rhs, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Operation(enc, InstructionsEnum.INCL, [lhs,rhs], options))

def equivalence(lhs, rhs, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Operation(enc, InstructionsEnum.EQUIV, [lhs,rhs], options))

def runCommand(operation):
    return vata_executor.runVata( \
            command_serializer.serializeCommand(operation))

from cli_options_enums import InstructionsEnum, EncodingsEnum
from cli_operation_representation import Command
import vata_executor
import command_serializer

def load(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Command(enc, InstructionsEnum.LOAD, [aut]))

def complement(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Command(enc, InstructionsEnum.CMPL, [aut]))

def witness(aut, enc=EncodingsEnum.EXPL):
   return runCommand(Command(enc, InstructionsEnum.WITNESS, [aut]))

def intersection(lhs,rhs, enc=EncodingsEnum.EXPL):
    return runCommand(Command(enc, InstructionsEnum.ISECT, [lhs,rhs]))

def union(lhs, rhs, enc=EncodingsEnum.EXPL):
    return runCommand(Command(enc, InstructionsEnum.UNION, [lhs,rhs]))

def simulation(aut, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Command(aut, InstructionsEnum.SIM, [aut], options))

def reduction(aut, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Command(aut, InstructionsEnum.RED, [aut], options))

def inclusion(lhs,rhs, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Command(enc, InstructionsEnum.INCL, [lhs,rhs], options))

def equivalence(lhs, rhs, enc=EncodingsEnum.EXPL, options=None):
    return runCommand(Command(enc, InstructionsEnum.EQUIV, [lhs,rhs], options))

def runCommand(operation):
    return vata_executor.runVata(command_serializer.serializeCommand(operation))

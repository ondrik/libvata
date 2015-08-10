import cli_operation_representation
import cli_options_enums

ENCODING_PARAM = '-r'
OPTION_PARAM = '-o'

def serializeEncoding(encoding):
    return [ENCODING_PARAM, cli_options_enums.EncodingToString[encoding]]

def serializeOperation(operation):
    return [cli_options_enums.InstructionsToStringCommand[operation]]

def serializeOperands(operands):
    return operands

def serializeOptions(command, options):
    res = options.serialize() \
            if command != cli_options_enums.InstructionsEnum.SIM else \
            options.serialize(command.getEncoding())
    
    return [OPTION_PARAM, res]

def serializeCommand(command):
    res = serializeEncoding(command.getEncoding())
    res += serializeOperation(command.getOperation())
    if command.getOptions() is not None:
        res +=  serializeOptions(command.getOptions())
    res += serializeOperands(command.getOperands())
    
    return res

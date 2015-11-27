import cli_options_enums

ENCODING_PARAM = '-r'
OPTION_PARAM = '-o'

def __serializeEncoding(encoding):
    return [ENCODING_PARAM, cli_options_enums.EncodingToString[encoding]]

def __serializeOperation(operation):
    return [cli_options_enums.OperationsToStringCommand[operation]]

def __serializeOperands(operands):
    return operands

def __serializeOptions(command, options):
    res = options.serialize() \
            if command != cli_options_enums.OperationsEnum.SIM else \
            options.serialize(command.getEncoding())

    return [OPTION_PARAM, res]

def serializeCommand(command):
    """
        Serialize @p command to a list of strings that can be further used for
        an execution of VATA CLI.
        @param command ... parameter representing command to be execture.
            It is supposed to be the class Command from
            module cli_operation_representation.
        @return A list of strings representing the command from parameter
    """
    res = __serializeEncoding(command.getEncoding())
    res += __serializeOperation(command.getOperation())
    if command.getOptions() is not None:
        res += __serializeOptions(command, command.getOptions())
    res += __serializeOperands(command.getOperands())

    return res

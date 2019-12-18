import os
import sys

try:
    script_path = os.path.dirname(os.path.abspath(__file__))
except:
    script_path = os.getcwd()

if __name__ == "__main__":
    cmd_line_args = sys.argv[1:]
    proj, build, arch = cmd_line_args

    hex_path_relative = ['..', '{}.cydsn'.format(proj), 'CortexM3', arch,
                        build, '{}.hex'.format(proj)]
    write_path_relative = ['batch-program']

    hex_path = os.path.join(script_path, *hex_path_relative)
    write_path = os.path.join(script_path, *write_path_relative)

    lines = [
        'OpenPort KitProg/0E230218001E6400 "c:/Program Files (x86)/Cypress/Programmer"',
        'HEX_ReadFile {}'.format(hex_path),
        'SetAcquireMode "Power"',
        'Acquire',
        'Calibrate',
        'EraseAll',
        'Program',
        'Verify',
        'VerifyProtect',
        'Protect',
        'CheckSum 0',
        'HEX_ReadChecksum'
    ]

    with open(write_path, 'w') as outfile:
        outfile.writelines(lines)

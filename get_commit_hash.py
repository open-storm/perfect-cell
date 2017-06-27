import os
import sys
import getopt

try:
    script_path = os.path.dirname(os.path.abspath(__file__))
except:
    script_path = os.getcwd()

log_path_relative = ['.git','logs', 'HEAD']
header_path_relative = ['perfect-cell.cydsn', 'commit.h']

log_path = os.path.join(script_path, *log_path_relative)
header_path = os.path.join(script_path, *header_path_relative)
ignore = False

def get_last_commit_info(log_path, ignore=False):
    """
    Get info for last commit from git logs

    log_path : Path to read log
    ignore : If True, do not read log. Writes default values to header file.
    """

    commit_info = {}

    # Default commit info
    prev_commit = 40*'0'
    current_commit = 40*'0'
    if os.path.exists(log_path):
        with open(log_path, 'r') as head:
            lines = head.readlines()
        info, message = lines[-1].split('\t')
        print(info)
        prev_commit, current_commit = info.split()[:2]
    else:
        if not ignore:
            raise OSError('''Git log file not found. Please clone repository
                          using git:
                          git clone https://github.com/open-storm/perfect-cell
                          or use the --ignore flag''')
    commit_info['PREVIOUS_COMMIT'] = prev_commit
    commit_info['CURRENT_COMMIT'] = current_commit
    return commit_info

def write_commit_header(header_path, commit_info):
    """
    Write current git commit info to header file

    header_path : Path to write header file
    commit_info : Dict containing header info
    """

    with open(header_path, 'w') as header:
        for name, value in commit_info.items():
            header.write("#define {0} \"{1}\"\n".format(name, value))
    return True

if __name__ == "__main__":
    cmd_line_args = sys.argv[1:]
    opts, remainder = getopt.getopt(cmd_line_args, 's:d:i', ['source=', 'dest=',
                                                             'ignore'])
    for opt, arg in opts:
        if opt in ('-s', '--source'):
            log_path = arg
        if opt in ('-d', '--dest'):
            header_path = arg
        if opt in ('-i', '--ignore'):
            ignore = True
    commit_info = get_last_commit_info(log_path, ignore=ignore)
    write_commit_header(header_path, commit_info)

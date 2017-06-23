import os
import sys

if __file__:
    script_path = os.path.dirname(os.path.abspath(__file__))
else:
    script_path = os.get_cwd()

log_path_relative = ['.git','logs', 'HEAD']
header_path_relative = ['perfect-cell.cydsn', 'commit.h']

log_path = os.path.join(script_path, *log_path_relative)
header_path = os.path.join(script_path, *header_path_relative)

def get_last_commit_info(log_path):
    commit_info = {}
    with open(log_path, 'r') as head:
        lines = head.readlines()
    info, message = lines[-1].split('\t')
    prev_commit, current_commit, author, email, timestamp, tz = info.split()
    commit_info['PREVIOUS_COMMIT'] = prev_commit
    commit_info['CURRENT_COMMIT'] = current_commit
    commit_info['AUTHOR'] = author
    commit_info['EMAIL'] = email
    commit_info['COMMIT_TIME'] = timestamp
    commit_info['COMMIT_TZ'] = tz
    commit_info['COMMIT_MESSAGE'] = message.strip()
    return commit_info

def write_commit_header(header_path, commit_info):
    with open(header_path, 'w') as header:
        for name, value in commit_info.items():
            header.write("#define {0} \"{1}\"\n".format(name, value))
    return True

if __name__ == "__main__":
    cmd_line_args = sys.argv
    if len(cmd_line_args) > 1:
    	log_path = cmd_line_args[1]
    if len(cmd_line_args) > 2:
    	header_path = cmd_line_args[2]
    commit_info = get_last_commit_info(log_path)
    write_commit_header(header_path, commit_info)
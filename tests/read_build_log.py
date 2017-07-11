import re
import sys
import datetime
from systemd import journal
sys.stdout.flush()

current_time = datetime.datetime.utcnow()

filters = 'ci_test|python-requests|WHERE commit_hash|^\s*service=httpd'
regex = re.compile(filters)
filter_on = True
filtered = False

if __name__ == "__main__":
    build_time = sys.argv[1]
    build_datetime = datetime.datetime.strptime(build_time,
                                            "%Y-%m-%d %H:%M:%S")
    print('Dumping influxdb logs...')
    print('Filters: {0}'.format(filters))
    with journal.Reader() as j:
        j.this_boot()
        j.log_level(journal.LOG_INFO)
        j.add_match(_SYSTEMD_UNIT="influxdb.service")
        j.seek_realtime(build_datetime)
        entry = j.get_next()
        while entry:
            msg = entry['MESSAGE']
            if filter_on:
                filtered = regex.search(msg)
            if not filtered:
                print(msg)
            entry = j.get_next()

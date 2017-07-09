import sys
import datetime
from systemd import journal

current_time = datetime.datetime.utcnow()

if __name__ == "__main__":
    build_time = sys.argv[1]
    build_time = datetime.datetime.strptime(build_time, "%Y-%m-%d %H:%M:%S")
    with journal.Reader() as j:
        j.this_boot()
        j.log_level(journal.LOG_INFO)
        j.add_match(_SYSTEMD_UNIT="influxdb.service")
        j.seek_realtime(build_time)
        entry = j.get_next()
        while entry:
            print(entry['MESSAGE'])
            entry = j.get_next()

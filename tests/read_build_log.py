import sys
import datetime
from systemd import journal

current_time = datetime.datetime.utcnow()

if __name__ == "__main__":
    build_date = sys.argv[1]
    build_time = sys.argv[2]
    build_timestamp = datetime.datetime.strptime(build_date + ' ' + build_time,
                                            "%Y-%m-%d %H:%M:%S")
    with journal.Reader() as j:
        j.this_boot()
        j.log_level(journal.LOG_INFO)
        j.add_match(_SYSTEMD_UNIT="influxdb.service")
        j.seek_realtime(build_timestamp)
        entry = j.get_next()
        while entry:
            print(entry['MESSAGE'])
            entry = j.get_next()

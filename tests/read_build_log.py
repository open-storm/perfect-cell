import sys
import datetime
from systemd import journal

current_time = datetime.datetime.utcnow()

if __name__ == "__main__":
    build_time = sys.argv[1]
    build_datetime = datetime.datetime.strptime(build_time,
                                            "%Y-%m-%d %H:%M:%S")
    with journal.Reader() as j:
        j.this_boot()
        j.log_level(journal.LOG_INFO)
        j.add_match(_SYSTEMD_UNIT="influxdb.service")
        j.seek_realtime(build_datetime)
        entry = j.get_next()
        while entry:
            print(entry['MESSAGE'])
            # sys.stdout.flush()
            entry = j.get_next()

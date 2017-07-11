import sys
import datetime
import influxdb

# Database information
host = "ec2-13-58-145-29.us-east-2.compute.amazonaws.com"
port = 8086
username = 'test_user'
password = 'test_pass'
database = 'TEST_DB'
ssl_enabled = True
node_id = 'ARB000'
fast_sleeptimer = 20
slow_sleeptimer = 4600
source = 'ci_test'
offset_seconds = 60
minimum_delay = 15
date_format = "%Y-%m-%d %H:%M:%S"

# Target variables and acceptable ranges
var_ranges = {
    'cell_fer' : [0, 99],
    'cell_strength' : [0, 100],
    'conn_attempts' : [1, 1000],
    'v_bat' : [-1, 10],
    'maxbotix_depth' : [0, 9998],
    'maxbotix_2_depth' : [0, 9998],
    'decagon_soil_conduct' : [0, 9998],
    'decagon_soil_dielec' : [0, 9998],
    'decagon_soil_temp' : [-40, 60],
    'atlas_dissolved_oxygen' : [0, 20],
    # 'gps_latitude' : [42.2, 42.4],
    # 'gps_longitude' : [-83.8, -83.6],
    # 'gps_altitude' : [200, 300],
    # 'gps_hdop' : [-9999, 9999],
    # 'gps_fix' : [-9999, 9999],
    # 'gps_cog' : [-9999, 9999],
    # 'gps_spkm' : [0, 9999],
    # 'gps_spkn' : [0, 9999],
    # 'gps_nsat' : [0, 12]
}

# Variables associated with a trigger
trigger_vars = {
    'gps_latitude' : 'gps_trigger',
    'gps_longitude' : 'gps_trigger',
    'gps_hdop' : 'gps_trigger',
    'gps_altitude' : 'gps_trigger',
    'gps_fix' : 'gps_trigger',
    'gps_cog' : 'gps_trigger',
    'gps_spkm' : 'gps_trigger',
    'gps_spkn' : 'gps_trigger',
    'gps_nsat' : 'gps_trigger',
    'isco_bottle' : 'autosampler_trigger'
}

# Store test results here
result_check = {}

# Get commit hash and build timestamp
git_commit = sys.argv[1]
# Timestamp requires Jenkins Build Timestamp plugin
build_timestamp = sys.argv[2]

# Offset timestamp if desired
if offset_seconds:
    build_datetime = datetime.datetime.strptime(build_timestamp, date_format)
    build_datetime = build_datetime + datetime.timedelta(seconds=offset_seconds)
    build_timestamp = datetime.datetime.strftime(build_datetime, date_format)

if __name__ == '__main__':
    # Instantiate client
    client = influxdb.InfluxDBClient(host=host, port=port, username=username,
                                    password=password, database=database, ssl=ssl_enabled)

    # Set a fast sleeptimer
    sleeptimer_query = ("SELECT last(value) FROM sleeptimer WHERE "
                        "node_id={0} AND source={1}").format(node_id, source)
    sleeptimer_result = client.query(sleeptimer_query)
    if sleeptimer_result:
        value_index = sleeptimer_result['columns'].index('last')
        sleeptimer_value = sleeptimer_result['values'][0][value_index]
    if (not sleeptimer_result) or (sleeptimer_value != fast_sleeptimer):
        client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                             .format(node_id, source, fast_sleeptimer)],
                            protocol='line')

    # Create query
    query_str = (("SELECT last(value) FROM {0} WHERE commit_hash=\'{1}\' AND "
                 "time > \'{2}\'")
                 .format(','.join(var_ranges.keys()),
                         git_commit,
                         build_timestamp))

    # Get query result
    result = client.query(query_str)


    # Check that all values lie within specified ranges
    for var in result.raw['series']:
        var_name = var['name']
        value_index = var['columns'].index('last')
        var_value = var['values'][0][value_index]
        print(var_name,':',var_value)
        try:
            assert(var_value >= var_ranges[var_name][0])
            assert(var_value <= var_ranges[var_name][1])
        except:
            # If rejected variable is associated with a trigger, set trigger high
            # for next try
            if var in trigger_vars:
                trigger = trigger_vars[var]
                client.write_points(['{0},node_id={1},source={2} value=1'
                                     .format(trigger, node_id, source)],
                                     protocol='line')
            # Raise an error if value is not in acceptable range
            raise ValueError(("{0} does not lie within specified"
                             "range").format(var))
        # Write passing variable to dictionary
        result_check.update({var_name : var_value})

    # Check that all requested variables were received
    result_vars = set(result_check.keys())
    target_vars = set(var_ranges.keys())
    try:
        assert(result_vars == target_vars)
    except:
        # Determine variables that are missing
        missing_vars = target_vars.difference(result_vars)
        # Determine trigger variables that are missing
        missing_trigger_vars = missing_vars.intersection(
            set(trigger_vars.keys()))
        # If trigger variables are missing, set triggers high for next try
        if missing_trigger_vars:
            missing_triggers = set([trigger_vars[var] for var in
                                    missing_trigger_vars])
            client.write_points(['{0},node_id={1},source={2} value=1'
                                 .format(trigger, node_id, source) for
                                trigger in missing_triggers], protocol='line')
        # Raise an error if not all requested variables were returned
        raise KeyError("Did not write variable(s) {0} to database"
                       .format(', '.join(missing_vars)))

    # Set slow sleeptimer
    client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                         .format(node_id, source, slow_sleeptimer)], protocol='line')

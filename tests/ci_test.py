import os
import sys
import datetime
import yaml
import influxdb

parent_path = os.path.abspath(os.path.join
                              (os.path.abspath(__file__),os.pardir))
config_path = os.path.join(parent_path, 'build_info.yaml')

# Read config info
config = {}
with open(config_path, 'r') as stream:
    config.update(yaml.load(stream))

# Store test results here
result_check = {}

# Get commit hash and build timestamp
git_commit = sys.argv[1]
# Timestamp requires Jenkins Build Timestamp plugin
build_timestamp = sys.argv[2]

# Offset timestamp if desired
if config['offset_seconds']:
    build_datetime = datetime.datetime.strptime(build_timestamp,
                                                config['date_format'])
    build_datetime = build_datetime + datetime.timedelta(
        seconds=config['offset_seconds'])
    build_timestamp = datetime.datetime.strftime(build_datetime,
                                                 config['date_format'])

if __name__ == '__main__':
    # Instantiate client
    client = influxdb.InfluxDBClient(host=config['host'], port=config['port'],
                                     username=config['username'],
                                     password=config['password'],
                                     database=config['database'],
                                     ssl=config['ssl_enabled'])

    # Set a fast sleeptimer
    sleeptimer_query = ("SELECT last(value) FROM sleeptimer WHERE "
                        "node_id={0} AND source={1}").format(config['node_id'],
                                                             config['source'])
    sleeptimer_result = client.query(sleeptimer_query)
    if sleeptimer_result:
        value_index = sleeptimer_result['columns'].index('last')
        sleeptimer_value = sleeptimer_result['values'][0][value_index]
    if (not sleeptimer_result) or (sleeptimer_value !=
                                   config['fast_sleeptimer']):
        client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                             .format(config['node_id'], config['source'],
                                     config['fast_sleeptimer'])],
                            protocol='line')

    # Create query
    query_str = (("SELECT last(value) FROM {0} WHERE commit_hash=\'{1}\' AND "
                 "time > \'{2}\'")
                 .format(','.join(config['var_ranges'].keys()),
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
            assert(var_value >= config['var_ranges'][var_name][0])
            assert(var_value <= config['var_ranges'][var_name][1])
        except:
            # If rejected variable is associated with a trigger, set trigger high
            # for next try
            if var_name in config['trigger_vars']:
                trigger = config['trigger_vars'][var_name]
                client.write_points(['{0},node_id={1},source={2} value=1'
                                     .format(config['trigger'],
                                             config['node_id'],
                                             config['source'])],
                                     protocol='line')
            # Raise an error if value is not in acceptable range
            raise ValueError(("{0} does not lie within specified"
                             "range").format(var_name))
        # Write passing variable to dictionary
        result_check.update({var_name : var_value})

    # Check that all requested variables were received
    result_vars = set(result_check.keys())
    target_vars = set(config['var_ranges'].keys())
    try:
        assert(result_vars == target_vars)
    except:
        # Determine variables that are missing
        missing_vars = target_vars.difference(result_vars)
        # Determine trigger variables that are missing
        missing_trigger_vars = missing_vars.intersection(
            set(config['trigger_vars'].keys()))
        # If trigger variables are missing, set triggers high for next try
        if missing_trigger_vars:
            missing_triggers = set([config['trigger_vars'][var] for var in
                                    missing_trigger_vars])
            client.write_points(['{0},node_id={1},source={2} value=1'
                                 .format(trigger,
                                         config['node_id'],
                                         config['source']) for
                                trigger in missing_triggers], protocol='line')
        # Raise an error if not all requested variables were returned
        raise KeyError("Did not write variable(s) {0} to database"
                       .format(', '.join(missing_vars)))

    # Set slow sleeptimer
    client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                         .format(config['node_id'],
                                 config['source'],
                                 config['slow_sleeptimer'])], protocol='line')

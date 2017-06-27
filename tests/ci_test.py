import os
import sys
import influxdb

host = "ec2-52-87-156-130.compute-1.amazonaws.com"
port = 8086
username = 'home_user'
password = 'home_password'
database = '_TEST'
ssl_enabled = True

var_ranges = {
    'cell_fer' : [0, 99],
    'cell_strength' : [0, 100],
    'conn_attempts' : [1, 1000],
    'v_bat' : [1, 10]
}

# Store test results here
result_check = {}

# Get commit hash and build timestamp
#git_commit = os.environ['GIT_COMMIT']
git_commit = sys.argv[1]
# Timestamp requires Jenkins Build Timestamp plugin
#build_timestamp = os.environ['BUILD_TIMESTAMP']
build_timestamp = sys.argv[2]

if __name__ == '__main__':
    # Instantiate client
    client = influxdb.InfluxDBClient(host=host, port=port, username=username,
                                    password=password, database=database, ssl=ssl_enabled)

    # Create query
    query_str = ('''SELECT last(value) FROM {0} WHERE commit_hash=\'{1}\' AND
                 time > \'{2}\' '''
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
        assert(var_value >= var_ranges[var_name][0])
        assert(var_value <= var_ranges[var_name][1])
        result_check.update({var_name : var_value})

    # Check that all requested variables were received
    assert(set(result_check.keys()) == set(var_ranges.keys()))

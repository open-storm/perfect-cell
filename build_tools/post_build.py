import os
import yaml
import sys
import influxdb

parent_path = os.path.abspath(os.path.join
                              (os.path.abspath(__file__),os.pardir))
config_path = os.path.join(parent_path, 'build_info.yaml')

# Read config info
config = {}
with open(config_path, 'r') as stream:
    config.update(yaml.load(stream))


if __name__ == '__main__':
    build_status = sys.argv[1].lower()

    # Instantiate client
    client = influxdb.InfluxDBClient(host=config['host'],
                                     port=config['port'],
                                     username=config['username'],
                                     password=config['password'],
                                     database=config['database'],
                                     ssl=config['ssl_enabled'])
    # Set slow sleeptimer
    client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                         .format(config['node_id'], config['source'],
                                 config['slow_sleeptimer'])], protocol='line')

    # Set sleeptimer mode to auto
    client.write_points(['sleeptimer_mode,node_id={0},source={1} value={2}'
                         .format(config['node_id'], config['source'],
                                 "auto")], protocol='line')

    # Set all triggers low
    trigger_vars = list(set(config['trigger_vars'].values()))
    client.write_points(['{0},node_id={1},source={2} value=0'
                         .format(config['trigger'], config['node_id'],
                                 config['source']) for
                         trigger in trigger_vars], protocol='line')

    # Write build status
    build_status_code = config['build_status_codes'][build_status]
    client.write_points([
        'build_status_code,node_id={0},source={1},status_description={2} value={3}'
        .format(config['node_id'], config['source'], build_status,
                build_status_code)], protocol='line')


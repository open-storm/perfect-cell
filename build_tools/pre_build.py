import os
import yaml
import influxdb

parent_path = os.path.abspath(os.path.join(
    os.path.abspath(os.path.join(
        os.path.realpath(__file__),os.pardir)),
    os.pardir))

config_path = os.path.join(parent_path, 'build_info.yaml')

# Read config info
config = {}
with open(config_path, 'r') as stream:
    config.update(yaml.load(stream))


if __name__ == '__main__':

    # Instantiate client
    client = influxdb.InfluxDBClient(host=config['host'],
                                     port=config['port'],
                                     username=config['username'],
                                     password=config['password'],
                                     database=config['database'],
                                     ssl=config['ssl_enabled'])
    # Set fast sleeptimer
    client.write_points(['sleeptimer,node_id={0},source={1} value={2}'
                         .format(config['node_id'], config['source'],
                                 config['fast_sleeptimer'])], protocol='line')

    # Set sleeptimer mode to manual
    client.write_points(['sleeptimer_mode,node_id={0},source={1} value={2}'
                         .format(config['node_id'], config['source'],
                                 "\"manual\"")], protocol='line')

    # Set all triggers high
    #trigger_vars = list(set(config['trigger_vars'].values()))
    #client.write_points(['{0},node_id={1},source={2} value=1'
    #                     .format(trigger, config['node_id'],
    #                             config['source']) for
    #                     trigger in trigger_vars], protocol='line')


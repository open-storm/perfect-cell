import os
import sys
import datetime
import yaml
import influxdb
import pandas as pd

parent_path = os.path.abspath(os.path.join(
    os.path.abspath(os.path.join(
        os.path.realpath(__file__),os.pardir)),
    os.pardir))

config_path = os.path.join(parent_path, 'build_info.yaml')

# Read config info
config = {}
with open(config_path, 'r') as stream:
    config.update(yaml.load(stream))

client = influxdb.DataFrameClient(host=config['host'], port=config['port'],
                                     username=config['username'],
                                     password=config['password'],
                                     database=config['database'],
                                     ssl=config['ssl_enabled'])

if __name__ == "__main__":

    commit_hash = sys.argv[1]

    for fn in os.listdir(parent_path):
        if fn.startswith('scope_01') and fn.endswith('bin'):
            datetime_str = fn.split('.')[0].split('_')[2:].replace('-', '.')
            sample_time = datetime.datetime.strptime(datetime_str, '%Y%m%d_%H%M%S.%f')
            dt_index = pd.date_range(start=sample_time, periods=len(data), freq='1ms')
            data = np.fromfile(fn, dtype=np.float32)
            series = pd.Series(data, index=dt_index)
            series = pd.DataFrame(series, columns=['value'])
            client.write_points(series, measurement='power_consumption',
                protocol='line', tags={'commit_hash' : commit_hash, 'source' : ci_test})

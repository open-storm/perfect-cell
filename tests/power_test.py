# Based on OscilloscopeStream.py - for LibTiePie 0.5+

from __future__ import print_function
import time
import datetime
import os
import sys
import yaml
import pandas as pd
import influxdb
import libtiepie

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

    # Print library info:
    # print_library_info()

    # Search for devices:
    libtiepie.device_list.update()

    # Try to open an oscilloscope with stream measurement support:
    scp = None
    for item in libtiepie.device_list:
        if item.can_open(libtiepie.DEVICETYPE_OSCILLOSCOPE):
            scp = item.open_oscilloscope()
            if scp.measure_modes & libtiepie.MM_STREAM:
                break
            else:
                scp = None

    if scp:
        try:
            # Set measure mode:
            scp.measure_mode = libtiepie.MM_STREAM

            # Set sample frequency:
            scp.sample_frequency = 1e3  # 1 kHz

            # Set record length:
            scp.record_length = 1000  # 1 kS

            # For all channels:
            for ch in scp.channels:
                # Enable channel to measure it:
                ch.enabled = True

                # Set range:
                ch.range = 8  # 8 V

                # Set coupling:
                ch.coupling = libtiepie.CK_DCV  # DC Volt

            # Print oscilloscope info:
            # print_device_info(scp)

            # Start measurement:
            scp.start()

            master_dataframe = pd.DataFrame(columns=['value'])
            try:

                # Measure 200 blocks:
                print()
                sample = 0
                for block in range(120):
                    if (block % 10 == 0):
                        print('Block: ', block)

                    # Wait for measurement to complete:
                    while not (scp.is_data_ready or scp.is_data_overflow):
                        time.sleep(0.01)  # 10 ms delay, to save CPU time

                    if scp.is_data_overflow:
                        print('Data overflow!')
                        break

                    # Get data:
                    data = scp.get_data()
                    series = pd.Series(data[0], index=pd.date_range(start=datetime.datetime.utcnow() - datetime.timedelta(seconds=1), 
                                       freq='1ms', periods=len(data[0])))
                    dataframe = pd.DataFrame(series, columns=['value'])
                    master_dataframe = master_dataframe.append(dataframe)
            except Exception as e:
                print('Exception: ' + e.message)

            # Stop stream:
            scp.stop()
            master_dataframe = master_dataframe.resample('2ms').mean().dropna()
            print('Writing power consumption measurements to influxdb...')
            client.write_points(master_dataframe, measurement='power_consumption',
                    time_precision='n', protocol='line', tags={'commit_hash' : commit_hash,
                                                               'source' : 'ci_test'})

        except Exception as e:
            print('Exception: ' + e.message)
            sys.exit(1)

        # Close oscilloscope:
        del scp

    else:
        print('No oscilloscope available with stream measurement support!')
        sys.exit(1)

    sys.exit(0)

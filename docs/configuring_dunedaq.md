# Configuring DUNE DAQ

First, while not strictly necessary, I recommend navigating to your $DBT_AREA_ROOT and creating a directory for your DUNE DAQ runs:

```bash
cd $DBT_AREA_ROOT
mkdir run
cd run
```

Now, we generate a DUNE DAQ run configuration using the `fddaqconf_gen` function that gets loaded when you source your env.sh script. There are plenty of example configuration files in the [fddaq-v4.4.8 setup instructions](https://github.com/DUNE-DAQ/daqconf/wiki/Setting-up-a-fddaq%E2%80%90v4.4.8-software-area); the one we're using is largely based on the "Sample daqconf.json for running with TPG" (Trigger Primitive Generation) but with a few modifications to tell it how to load and configure the Triton plugin from triggeralgs. Copy the text below and save it as something like `daqconf.json`:

```json
{
  "boot": {
    "use_connectivity_service": true,
    "start_connectivity_service": true,
    "connectivity_service_host": "localhost",
    "connectivity_service_port": 15432
  }, 
  "daq_common": {
    "data_rate_slowdown_factor": 1
  },
  "detector": {
    "clock_speed_hz": 62500000,
    "offline_data_stream": "cosmics"
  },
  "readout": {
    "use_fake_cards": true,
    "default_data_file": "asset://?checksum=dd156b4895f1b06a06b6ff38e37bd798",
    "generate_periodic_adc_pattern": true,
    "emulated_TP_rate_per_ch": 1,
    "enable_tpg": true,
    "tpg_threshold_default": 500,
    "tpg_algorithm": "SimpleThreshold"
  },
  "trigger": {
    "ttcm_input_map": [{"signal": 1, "tc_type_name": "kTiming",
                        "time_before": 1000, "time_after": 1000}],
    "trigger_activity_plugin": ["TriggerActivityMakerTritonPlugin"],
    "trigger_activity_config": [ {"number_tps_per_request":      100, 
                                  "batch_size":                  1,
                                  "number_time_ticks":           128,
                                  "number_wires":                128,
                                  "inference_url":               "localhost:8001",
                                  "model_name":                  "simple",
                                  "model_version":               "1",
                                  "client_timeout_microseconds": 5000,
                                  "verbose":                     true,
                                  "outputs":                     [[]],
                                  "print_tp_info":               true} ],
    "trigger_candidate_plugin": ["TriggerCandidateMakerPrescalePlugin"],
    "trigger_candidate_config": [ {"prescale": 100} ],
    "mlt_merge_overlapping_tcs": false
  },
  "dataflow": {
    "apps": [ { "app_name": "dataflow0" } ],
    "enable_tpset_writing": true,
    "token_count": 20
  },
  "hsi": {
    "random_trigger_rate_hz": 1.0
  }
}
```

Note the `TriggerActivityMakerTritonPlugin` under the trigger section and its associated parameters. Note also that this configuration turns on trigger primitive generation, as seen by

```json
    "emulated_TP_rate_per_ch": 1,
    "enable_tpg": true,
    "tpg_threshold_default": 500,
    "tpg_algorithm": "SimpleThreshold"
```

You'll also need a detector readout (DRO) map file. Again, there are many examples in the setup instructions, but for now you can just copy the code below and save it as something like `dro_map.json`:

```json
[
    {
        "src_id": 100,
        "geo_id": {
            "det_id": 3,
            "crate_id": 1,
            "slot_id": 0,
            "stream_id": 0
        },
        "kind": "eth",
        "parameters": {
            "protocol": "udp",
            "mode": "fix_rate",
            "rx_iface": 0,
            "rx_host": "localhost",
            "rx_pcie_dev": "0000:00:00.0",
            "rx_mac": "00:00:00:00:00:00",
            "rx_ip": "0.0.0.0",
            "tx_host": "localhost",
            "tx_mac": "00:00:00:00:00:00",
            "tx_ip": "0.0.0.0"
        }
    },
    {
        "src_id": 101,
        "geo_id": {
            "det_id": 3,
            "crate_id": 1,
            "slot_id": 0,
            "stream_id": 1
        },
        "kind": "eth",
        "parameters": {
            "protocol": "udp",
            "mode": "fix_rate",
            "rx_iface": 0,
            "rx_host": "localhost",
            "rx_pcie_dev": "0000:00:00.0",
            "rx_mac": "00:00:00:00:00:00",
            "rx_ip": "0.0.0.0",
            "tx_host": "localhost",
            "tx_mac": "00:00:00:00:00:00",
            "tx_ip": "0.0.0.0"
        }
    }
]
```

With these files in place, you can generate your configuration by running

```bash
fddaqconf_gen -c daqconf.json --detector-readout-map-file dro_map.json <config_name>
```

The `config_name` can be whatever you want, but it should be descriptive of the configuration. For example, if you copy the above files exactly, you could name the configuration `triton_default`. Then, if you change a parameter, say by setting `batch_size` to 2, you would generate a separate configuration that could be called `triton_batch_size_2`. Obviously, this is just a suggestion; do whatever you want.

Whatever you name your configuration, you should now see a directory with that name in the area where you ran `fddaqconf_gen`. 

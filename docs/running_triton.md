# Running Triton in DUNE DAQ

## ssh Tunnel to ailab01 (or Somewhere with a Triton Server)

Before running the Triton client in DUNE DAQ, you'll need to tell it where to find the Triton server. I generally use the one on `ailab01` that runs continuously on port number 8001. If you use another node, simply replace `ailab01` and the port number below with your host name and port number. To open the ssh tunnel, open up a new terminal (or tmux, or screen, or whatever you want that can run in the background) and run

```
ssh -KL 8001:ailab01.fnal.gov:8001 $USER@ailab01.fnal.gov
```

This will forward the Triton process running on port 8001 on `ailab01` to the same port on whatever machine you're using. This way, the plugin should correctly recognize `localhost:8001` as the server port. 

> [!NOTE]
> If you get a `Permission denied (gssapi-keyex,gssapi-with-mic)` error, run `kinit $USER@FNAL.GOV` and enter your Kerberos password. If that doesn't work, you probably need to request access to the node you're trying to ssh into. If you need access to `ailab01.fnal.gov` but don't have it, contact Burt Holzman.

## Running the DAQ

The DAQ run control is handled by a package called `nanorc` ("not another run control"). The usage for our case is

```
nanorc --partition-number <num> <config_name> <partition name> boot conf start_run --run-type TEST <run number> wait <duration_in_seconds> stop_run scrap terminate
```

`--partition-number` can take any integer between 1 and 9, inclusive. The choice is somewhat arbitrary, as this number is only used to prevent port collisions with other users. The `<config_name>` needs to match the name of the configuration you generated , e.g., `triton_default` (see [Configuring DUNE DAQ](configuring_dunedaq.md) if you haven't already done this). The `partition-name` is arbitrary, as is `run_number` since we're just doing test runs with fake data.

For example, to do a 10 second run using a generated configuration named `triton_config`, run

```
nanorc --partition-number 7 triton_config test-session boot conf wait 10 start_run 111 wait 10 stop_run scrap terminate
```

> [!WARNING]
> Note that this creates a directory called `RunConf_<run_number>`. If you attempt another run with the same run number, nanorc will complain. Either delete your old directory or use a different run number to fix this.

You can see in the output, under the Looking for services line, that there are paths to output logs for each DAQ application. The trigger log (`log_trigger_<port_number>.txt` in this case) is where the Triton plugin writes its output, so that's your go-to debugging log. 

Congratulations, you've now run DUNE DAQ!

## Examining the Output HDF5 Files

The command in the previous step will output two HDF5 files, one whose name starts with `swtest` and another with `tpstream`. These correspond to the raw data stream and the trigger primitive stream, respectively. DUNE DAQ comes with two command-line tools to help examine these files:

```
HDF5LIBS_TestDumpRecord <filename>
h5dump-shared -H <filename>
```

`HDF5LIBS_TestDumpRecord` will print information about the objects stored in the file, such as TimeSlices and TriggerPrimitives:

```
TimeSliceHeader: check_word: 55556666, version: 2, timeslice_number: 1, run_number: 111, element_id: { subsystem: TR_Builder id: 1 }
Trigger_Primitive fragment with SourceID Trigger_0x00000000 from subdetector DAQ has size = 192600 -----
        Number of TPs in this fragment=3438, size of TP data structure=56, size of Fragment Header=72
        First TP type = 1, TP algorithm = 1
        First TP start time=79554162068740423, peak time=79554162068740423, and time over threshold=32
```

while `h5dump-shared` will print an overview of the file structure, i.e., the available groups and datasets. For our case, these tools are mostly useful for understanding how and what TP information is stored. 

## Using trace for Output Logging

`trace` is a low-memory-overhead logging package used in DUNE DAQ. Note that trace effectively replaces std::cout. You can read an overview in the setup instructions; here, I'm just going to talk about how to use it. 

First, to enable trace logging, you need to set the environment variable `TRACE_FILE` to point to your local area:

```
export TRACE_FILE=$DBT_AREA_ROOT/log/${USER}_dunedaq.trace
```

Note that this command needs to be re-run each time you start a new shell. You can then run tlvls to see the available names known to trace. For example,

```
mode:                                                        M=1                S=1
 TID                                     NAME              maskM              maskS              maskT
---- ---------------------------------------- ------------------ ------------------ ------------------
  12                             RestEndpoint 0x00000000000001ff 0x00000000000000ff 0x0000000000000000
  13                          AbstractFactory 0x00000000000001ff 0x00000000000000ff 0x0000000000000000
  30                          PD2HDChannelMap 0x00000000000001ff 0x00000000000000ff 0x0000000000000000
  34                                 TABuffer 0x00000000000001ff 0x00000000000000ff 0x0000000000000000
```

If you run `tlvls | grep Triton`, you should see something like

```
568               TriggerActivityMakerTriton 0x00000000000001ff 0x00000000000000ff 0x0000000000000000
```

To enable logging output from the Triton plugin, you need to select the level of output. You can see the available levels in triggeralgs/include/triggeralgs/Logging.hpp:

```
enum Logging
{
  TLVL_VERY_IMPORTANT = 1,
  TLVL_IMPORTANT      = 2,
  TLVL_GENERAL        = 3,
  TLVL_DEBUG_INFO     = 4,
  TLVL_DEBUG_LOW      = 5,
  TLVL_DEBUG_MEDIUM   = 10,
  TLVL_DEBUG_HIGH     = 15,
  TLVL_DEBUG_ALL      = 20
};
```

Basically, this means that if you want to enable, say, `TLVL_DEBUG_INFO` output, you run

```
tonS -n TriggerActivityMakerTritonPlugin DEBUG+4
```

Or, to enable `TLVL_DEBUG_ALL`, 

```
tonS -n TriggerActivityMakerTritonPlugin DEBUG+20
```

Then, for example, you would be able to see lines like

```
TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TAM:Triton] Server metadata debug string: " << server_metadata.DebugString();
```

printed to the trigger log file. 




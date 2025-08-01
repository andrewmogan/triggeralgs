# Contributing

To avoid stepping on each others' toes, pushing to the `main` branch of this repository requires you to open a pull request and get it approved by at least one person. Any development work should be performed on a dedicated branch. You should only open a pull request after you've confirmed that your work is compatible with the code on the `main` branch (run `git pull origin main` before submitting your PR). 

## Explanation of the Triton Plugin and How to Modify It

If you've read this far, you should be able to [install Triton](triton_installation.md), [configure DUNE DAQ with the Triton trigger algorithm enabled](configuring_dunedaq.md), and [run the Triton plugin](running_triton.md). But eventually, you may want to modify the trigger algorithm and/or add your own model. This section and the following ones explain how the Triton plugin works and how to modify it.

The "main" function lives in `triggeralgs/src/TriggerActivityMakerTriton.cpp`. The associated header file is located in `triggeralgs/include/triggeralgs/Triton/`. The "main" function definition in the .cpp file looks like

```c++
void
TriggerActivityMakerTriton::operator()(const TriggerPrimitive& input_tp, std::vector<TriggerActivity>& output_tas)
{ ... }
```

This is basically a boilerplate function that reads in TPs and (generally) packages them up in some way into a TriggerActivity (recall that a TA is just a std::vector of TPs). Technically, we're currently not creating TAs, but this is a convenient way to look at TPs and package them up for Triton inference. In its current state, this function waits for the configured number of TPs before doing anything:

```c++
if (m_current_ta.inputs.size() < m_number_tps_per_request) {
    m_current_ta.inputs.push_back(input_tp);
    return;
}
```

Once it has enough TPs, it attempts an inference request:

```c++
  const std::string model_name = triton_client->get_model_name();
  triton_client->set_batch_size(m_batch_size);

  const std::unordered_map<std::string, ModelIOHandler> handlers = triggeralgs::get_model_io_handlers();
  auto it = handlers.find(model_name);
  if (it == handlers.end()) {
    std::cerr << "No model IO handler registered for model: " << model_name << std::endl;
    exit(1);
  }

  ModelIOHandler handler = it->second;
  handler.prepare_input(*triton_client);

  triton_client->dispatch();

  handler.handle_output(*triton_client);

  triton_client->reset();
  m_current_ta = TriggerActivity();
  return;
```

Basically, the `model_name` parameter is picked up from your configuration and used by the `ModelIOHandler` class to point to the pre- and post-processing functions for that model. The `triton_client->dispatch()` command sends the actual inference request. If you copied the `daqconf.json` file from [the configuration instructions](configuring_dunedaq.md), the model you're querying is a toy model provided by NVIDIA called "simple". This model simply (heh) defines two vectors of integers and requests their sums and differences. The actual code that defines these vectors and formats them in a way that Triton can read can be found in `src/Triton/ModelIOHandler.cpp`. This file contains the `get_model_io_handlers` function that maps your `model_name` parameter to the pre- and post-processing functions:

```c++
const std::unordered_map<std::string, ModelIOHandler>& get_model_io_handlers() {
  static std::unordered_map<std::string, ModelIOHandler> handlers = {
    {
      "simple",
      {
        simple_input_preparer,
        simple_output_handler
      }
    },
```

I won't paste them here for brevity, but you can look at the `simple_input_preparer` and `simple_output_handler` functions further down in that file to see how they work. 

> [!NOTE]
> The important thing to understand is that, when adding a new model, you need to write pre- and post-processing functions associated with that `model_name`. The preprocessor should format Trigger Primitives into whatever data format your model requires. The postprocessor should handle your outputs however may be appropriate, such as simply printing the results to the log file.

The configure function in `src/TriggerActivityMakerTriton.cpp` sets the various configuration parameters defined in your `daqconf.json` file. For example, `m_number_tps_per_request` is set to the `number_tps_per_request` value from the config file:

```c++
  if (config.is_object() && config.contains("number_tps_per_request")) {
    m_number_tps_per_request = config["number_tps_per_request"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured TPs per request: " << m_number_tps_per_request;
  }
```

The (optional) `TLOG_DEBUG` macro is a function from the trace package which is used to print information from DUNE DAQ to various streams, such as INFO, WARNING, ERROR, etc. See the section below on how to enable and use trace output. 

The majority of the remaining code in the plugin is adapted from the examples in the triton-client repo linked above. You can see the available client functions in the grpc_client.h file, also located in the triton-client repo.   

## Trigger Primitive Config Parameters

You can find the code for most of these parameters in the daqconf repo. 

* enable_tpg: Enables trigger primitive generation (hence "tpg"). Should be true. 
* emulated_TP_rate_per_ch: Confusingly, the number here is secretly multiplied by 100, meaning that 1 means "100 Hz per channel." See the daqconf link above. 
* tpg_threshold_default: threshold of ADC counts above which TPs are generated. Default is 120, but I have this arbitrarily set to 500. 
* tpg_algorithm: algorithm used to generate TPs. I've only used SimpleThreshold, but there are fancier ones available.

## Explanation of Triton-Specific Parameters

* number_tps_per_request: the number of TPs to store before sending an inference request
* batch_size: currently set to 1, needs some studying
* number_time_ticks: the number of time ticks used in the offline images. Currently 128
* number_wires: the number of wires in the offline images. Currently 128
* inference_url: the URL to look for a running Triton server instance. Remember to set up the ssh tunnel like in the above instructions for this to work
* model_name: should match one of the models stored in the /models directory on ailab01, or wherever your Triton server runs
* model_version: this should also match what's in the /models directory
* print_tp_info: a verbose flag for printing trigger primitive information
* client_timeout_microseconds the amount of time the client will wait for a response from the server before timing out
* verbose: set to true for additional logging information
* outputs: an empty vector to store outputs from the inference request. TBD whether this will continue to work for models beyond simple :shrug::skin-tone-2: 

## Adding New Triton Config Parameters

Depending on your use case, you may want to add configuration parameters for use in your processing functions. In order for your new parameters to be recognized by the Triton plugin, they need to be added in a few places. First, the new parameter should be added to your daqconf.json file like the others:

```json
    "trigger_activity_config": [ {"number_tps_per_request": 10000, 
                                  "inference_url":          "localhost:8001",
                                  "model_name":             "simple",
                                  "my_new_param":           "Hello, world!",
```

You'll also need to add it in the header file sourcecode/triggeralgs/include/triggeralgs/Triton/TriggerActivityMakerTriton.hpp, similar to the other parameters there:

```c++
  private:
    uint64_t m_number_tps_per_request = 100;
    uint64_t m_batch_size = 1;
    uint64_t m_number_time_ticks = 128;
    uint64_t m_number_wires = 128;
    std::string m_inference_url = "localhost:8001";
    std::string m_my_new_param = "";
```

and in the .cpp file under the configuration function:

```c++
  if (config.is_object() && config.contains("my_new_param")) {
    m_my_new_param = config["my_new_param"];
    TLOG_DEBUG(TLVL_DEBUG_INFO) << "[TA:Triton] Configured batch size: " << m_batch_size;
  }
```

Finally, add it to sourcecode/daqconf/schema/daqconf/triggergen.jsonnet. Again, use what's already there as a template:

```
    s.field("number_tps_per_request", types.count, default=100),
    s.field("batch_size", types.count, default=1),
    s.field("my_new_param", types.string, default=""),
```

> [!NOTE]
> Remember to compile with dbt-build after modifying the code so that nanorc will pick up the changes. 

## Adding a New Model to the Triton Server

The Triton server checks for models in a predefined models repository. In order for your model to be available on the Triton server, you need to provide a model configuration and put it in the models repository. On `ailab01`, the models repository is the root-level `/models` directory. You can peruse that directory for examples of models currently deployed. If you want to add your model configuration to the models repository on `ailab01`, contact Burt Holzman.

Once your model is available on the server, you need to configure the DUNE DAQ Triton plugin to use that model and write pre- and post-processing functions to handle your inputs and outputs, as mentioned previously. For example, a preprocessing function might take the ADC counts from a trigger primitive and format them as a 1D vector that can be queried by a 1D CNN. 

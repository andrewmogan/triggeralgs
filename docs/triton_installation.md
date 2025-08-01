# Installing Triton in DUNE DAQ

## Creating a Local DUNE DAQ Area

To start, we need to create a development area based on DUNE DAQ v4.4.8, tagged as fddaq-v4.4.8 (fddaq is shorthand for "far detector DAQ", which is used by ProtoDUNE and Iceberg). The full instructions for setting up an area based on fddaq-v4.4.8-a9 are [here](https://github.com/DUNE-DAQ/daqconf/wiki/Setting-up-a-fddaq%E2%80%90v4.4.8-software-area), but some of those steps are overkill for our case. To start, run the following commands on a node with access to cvmfs (such as the gpvms) in a clean directory where you have write access (such as your /app space):

```bash
source /cvmfs/dunedaq.opensciencegrid.org/setup_dunedaq.sh
setup_dbt fddaq-v4.4.8
dbt-create -s fddaq-v4.4.8-a9 <work_dir>
```

That last command will create a local DUNE DAQ area in a directory named `<work_dir>` (or it will default to fddaq-v4.4.8-a9 if you don't supply a name). The `-s` creates a local spack area which we'll use for configuring Triton later. You can then set up your environment by running

```bash
cd <work_dir>
source env.sh
```

This sets up some important environment variables for building your local code, including an environment variable called `DBT_AREA_ROOT` which you can use to access the top of your local area at any time.

Next, you'll need to clone some Triton-specific forks of the `triggeralgs`, `daqconf` , and `trigger` repos of DUNE DAQ to your local sourcecode directory:

```bash
cd $DBT_AREA_ROOT/sourcecode
git clone https://github.com/andrewmogan/trigger.git
git clone https://github.com/andrewmogan/triggeralgs.git
git clone https://github.com/andrewmogan/daqconf.git
```

If you plan on making changes to the Triton plugin, switch to your own, new branch and work from there so as not to overwrite anything. You can do so by running

```bash
git switch -c your_username/my_new_branch
```

It should go without saying that your actual branch name should be descriptive of the work it contains. For example, `amogan/simple_model` would be a sufficient name.

## Installing Triton through Spack

If you try to build this local area as is by running `dbt-build`, it will fail because Triton hasn't been installed yet. To install Triton, we'll need a spack recipe file that tells spack how to install it. A Triton spack that works for our purpose exists in a fork of the `fnal_art` repo. To clone that repository and copy the necessary spack recipes, run

```bash
git clone https://github.com/andrewmogan/fnal_art.git
./fnal_art/scripts/copy_triton_package_to_dunedaq.sh
```

If successful, that script should output "Done". To check that your local spack environment has picked up the triton recipe, run 

```
spack list triton
```

The expected output is

```
triton
==> 1 packages
```

Ideally, we would now be able to run `spack install triton` and everything would magically work. Alas, there are some tricky dependencies that require specific build commands, namely protobuf and abseil-cpp. Basically, you need to run

```bash
spack install --reuse triton ^/lwk452l ^/tti6ovt
```

In case you're curious, those hashes come from the output of

```bash
spack find -l -d fddaq | grep abseil-cpp
spack find -l -d fddaq | grep protobuf
```

This shows us the specific hashes corresponding to versions of `abseil-cpp` and `protobuf` that will build with the `fddaq` "umbrella" package.

The `--reuse` flag isn't strictly necessary, but it saves time by telling spack not to reinstall packages that are already installed. Depending on which machine you run this on, it might take a while: on `daq.fnal.gov`, a machine with 128 cores, this took me about 10 minutes. A machine with fewer cores may take longer (run `lscpu` to see information on your machine's resources). Once it's done installing, run 

```
spack load triton
```

to make Triton available in your local environment. Now, you should be able to successfully build your local area by running

```
dbt-workarea-env # Update your environment to see your locally checked out packages
dbt-build
```

Now, every time you log in in the future, you simply need to `cd` to your local build area, then run

```
source env.sh
spack load triton
```

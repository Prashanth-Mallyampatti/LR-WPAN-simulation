# LR-WPAN-simulation
Multi Node 802.15.4 IEEE LR-WPAN model simulation

**NS3 Installation**
Following are the steps to install NS3 on Ubuntu 18.04:

* [*Pre-requisites*](https://www.nsnam.org/docs/tutorial/html/getting-started.html):

  1. gcc compiler
  2. Python 2 and 3
  3. Git
  4. tar
  
* *Installation*:

  Note: `root` permissions required for the following steps
  1. Clone the repository:<br/>
      `git clone https://githu  b.com/Prashanth-Mallyampatti/LR-WPAN-simulation.git`
  2. Change directory:<br/>
      `cd LR-WPAN-simulation/`
  3. Run the installation script:<br/>
      `bash ./NS3_installation/ns3_install.sh`

  After successful installation `ns-allinone-3.29/` directory would be created.
      
* *Execute the lr-wpan-data*:

  1. `cd ns-allinone-3.29/ns-3.29`<br/>
      This is where `waf` would be run to build the scripts. `scratch/` folder can be used to play around with modules and build new ones.
  2. Copy the repository folder to the NS3 folders: <br/>
      `cp -r LR-WPAN-simulation/lr-wpan LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/src/lr-wpan`
  3. Run the `lr-wpan-data` code using the follwowing command in `LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/` directory:<br/>
      `./waf --run "src/lr-wpan/examples/lr-wpan-data"`
      <br/><br/>
      Example to run with command line arguments:<br/>
      `./waf --run "src/lr-wpan/examples/lr-wpan-data --nSenders=5"`
  4. Trace files- PCAPs and ASCII placed in:<br/>
      `LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/LR-wpan-trace/` directory.
  5. Plot files placed in:<br/>
      `LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/` directory.
 
* *Execute the lr-wpan-error-distance-plot*:
  
  1. `cd ns-allinone-3.29/ns-3.29`<br/>
  2. Run the `lr-wpan-error-distance-plot` code using the follwowing command in `LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/` directory:<br/>
      `./waf --run "src/lr-wpan/examples/lr-wpan-error-distance-plot"`
      <br/><br/>
      Example to run with command line arguments:<br/>
      `./waf --run "src/lr-wpan/examples/lr-wpan-error-distance-plot --nSenders=5"`
  3. Plot files placed in:<br/>
      `LR-WPAN-simulation/ns-allinone-3.29/ns-3.29/` directory.

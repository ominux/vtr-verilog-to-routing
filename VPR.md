# VPR #

## Introduction ##

VPR is an open source academic CAD tool designed for the exploration of new FPGA architectures, at the packing, placement and routing phases of the CAD flow.  Since its public introduction more than 10 years ago, VPR has been used extensively in many academic projects partly because it is robust, well documented, easy-to-use, and flexible across a range of architectures.

VPR takes, as input, a description of a hypothetical FPGA architecture along with a technology-mapped user circuit.  It then performs packing, placement, and routing to map that circuit to that FPGA.  The output of VPR includes the FPGA configuration needed to implement the circuit and statistics about the final mapped design (eg. critical path delay, area, etc).

Since its inception, VPR has been under constant evolution to offer support for the ever growing space of interesting FPGA architectures.  This latest version of VPR, VPR 7.0, incorporates carry chain support, multiple clock timing analysis, faster packing and placement and power analysis, amongst other upgrades.

## Motivation ##

The study of FPGA CAD and architecture can be a challenging process partly because of the difficulty in conducting high quality experiments.  A quality CAD/architecture experiment requires realistic benchmarks, accurate architectural models, and robust CAD tools that can appropriately map the benchmark to the particular architecture in question.  This is a lot of work.  Fortunately, this work can be made easier if open source tools were available as a starting point.

The purpose of VPR is to make the packing, placement, and routing stages of the FPGA CAD flow robust and flexible so that it is easier for researchers to investigate future FPGAs.  It is part of a broader open source project called [VTR](http://code.google.com/p/vtr-verilog-to-routing/) where we provide a complete infrastructure (architectures, realistic benchmarks, and CAD tools) for performing FPGA CAD/architecture experiments.

## Downloads ##
  * Download VPR 7.0 release with the standard VTR release [here](http://www.eecg.utoronto.ca/vtr/terms.html)
    * If you are only interested in running VPR, you can use the pre-compiled blif files located in _vtr\_release/vtr\_flow/vtr\_benchmarks\_blif_ and the architecture file _vtr\_release/vtr\_flow/arch/timing/k6\_N10\_memDepth16384\_memData64\_40nm\_timing.xml_
  * Get the active trunk of VPR [here](http://code.google.com/p/vtr-verilog-to-routing/source/checkout) **Note that because this is in active development, it may be unstable**

## Change Log ##

### VPR 7.0 ###
  * Support for carry chains and dedicated, hard-wired routing
  * Power analysis
  * Multiple clock timing analysis (a subset of SDC is supported)
  * Faster packing and placement
  * Microsoft Windows graphics support added (and X windows maintained)

### VPR 6.0 ###
  * Timing-driven (the beta was not)
  * Faster routing
  * New command-line interface
### VPR 6.0 Beta ###
  * Ability to explore logic blocks with far more complexity than VPR 5.0
    * Configurable memories, multipliers, fracturable LUTs, and more
    * Logic blocks can be specified to have arbitrary hierarchy, modes of operation, and interconnect
  * Merged packing into VPR, deprecated T-VPack
### VPR 5.0 ###
  * Single-driver routing architectures
  * Heterogeneous blocks in placement-and-routing
  * Transistor-optimized architecture files

## History ##

For those interested in the legacy versions of VPR, you can access them below:

[VPR 5.0](http://www.eecg.utoronto.ca/vpr/)

[VPR 4.30](http://www.eecg.toronto.edu/~vaughn/vpr/vpr.html)

**Warning: We are only providing support for the latest version of VPR.  There is no support for legacy versions.**
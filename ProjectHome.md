# The Verilog-to-Routing (VTR) Project for FPGAs #

## NEW RELEASE AVAILABLE OCTOBER 7, 2013 ##
### [download](http://www.eecg.utoronto.ca/vtr/terms.html) ###


**Current Version: 7.0 Full Release** -- _last updated April 22, 2014_

Quick Links to Buildbot Site of Current Trunk  [Buildbot](http://islanders.eecg.utoronto.ca:8080)  [Build Waterfall](http://islanders.eecg.utoronto.ca:8080/waterfall)    [QoR Plots](http://islanders.eecg.utoronto.ca:8080/qor_basic.html)

## Introduction ##

The Verilog-to-Routing (VTR) project is a world-wide collaborative effort among multiple research groups to provide a complete, open-source framework for conducting FPGA architecture and CAD research and development. This software flow begins with a Verilog hardware description of digital circuits, and a file describing the target hypothetical architecture, and elaborates, synthesizes, packs, places and routes the circuit, and performs timing analysis on the result.

## Motivation ##

The study of new FPGA architectures/algorithms can be a difficult process partly because of the effort required to conduct quality experiments. A good FPGA architecture/algorithm experiment requires realistic benchmark circuits, optimized architectures, and CAD tools that can efficiently map those benchmark circuits to those architectures. The VTR project enables such experiments by providing FPGA architects with a flexible and robust CAD flow for FPGAs.

## Release ##

The VTR 7.0 release provides the following: benchmark circuits, sample FPGA architecture description files, a full CAD flow and scripts to run that flow. This FPGA CAD flow takes as input, a user circuit (coded in Verilog) and a description of the FPGA architecture. The CAD flow then maps the circuit to the FPGA architecture to produce, as output, a placed-and-routed FPGA. Here are some highlights of the 7.0 full release:

  * Timing-driven logic synthesis, packing, placement, and routing with multi-clock support.
  * Power Analysis
  * Benchmark digital circuits consisting of real applications that contain both memories and multipliers.    Seven of the 19 circuits contain more than 10,000 6-LUTs. The largest of which is just under 100,000 6-LUTs.
  * Sample architecture files of a wide range of different FPGA architectures including: 1) Timing annotated architectures 2) Various fracturable LUTs (dual-output LUTs that can function as one large LUT or two smaller LUTs with some shared inputs) 3) Various configurable embedded memories and multiplier hard blocks 4) One architecture containing embedded floating-point cores, and 5) One architecture with carry chains.
  * A front-end Verilog elaborator that has support for hard blocks. This tool can automatically recognize when a memory or multiplier instantiated in a user circuit is too large for a target FPGA architecture. When this happens, the tool can automatically split that memory/multiplier into multiple smaller components (with some glue logic to tie the components together). This makes it easier to investigate different hard block architectures because one does not need to modify the Verilog if the circuit instantiates a memory/multiplier that is too large.
  * Packing/Clustering support for FPGA logic blocks with widely varying functionality. This includes memories with configurable aspect ratios, multipliers blocks that can fracture into smaller multipliers, soft logic clusters that contain fracturable LUTs, custom interconnect within a logic block, and more.
  * Ready-to-run scripts that guide a user through the complexities of building the tools as well as using the tools to map realistic circuits (written in Verilog) to FPGA architectures.
  * Regression tests of experiments that we have conducted to help users error check and/or compare their work. Along with experiments for more conventional FPGAs, we also include an experiment that explores FPGAs with embedded floating-point cores investigated in [Ho2009](http://ieeexplore.ieee.org/xpl/freeabs_all.jsp?arnumber=4814468&tag=1) to illustrate the usage of the VTR framework to explore unconventional FPGA architectures.

The VTR 7.0 release announcement may be found here [VTR 7.0 Release Announcement](VTR_7_release_announcement.md).

The full VTR release may be downloaded [here](http://www.eecg.utoronto.ca/vtr/terms.html). To build and run the tool, unzip the archive and follow the instructions in vtr\_release/README.txt.

## Links ##

### [Download VTR 7.0 Release](http://www.eecg.utoronto.ca/vtr/terms.html) ###

### [Installing VTR](InstallingVTR.md) ###

### [Tutorials on Running VTR](RunningVTR.md) ###

### [Frequently Asked Questions](FAQ.md) ###

### [Wiki Table of Contents](TOC.md) ###

## Additional Information ##


The following sites describe the different tools within this flow:

  * [ODIN II](https://code.google.com/p/odin-ii/): Elaboration and synthesis tool.
  * [ABC](http://www.eecs.berkeley.edu/~alanmi/abc/): Logic synthesis and FPGA technology mapping tool (the one we provide is a custom made one that includes the wiremap algorithm. The wiremap algorithm is described [here](http://portal.acm.org/citation.cfm?id=1344671.1344680)).
  * [VPR 7.0](VPR.md): An FPGA packing, placement, and routing tool.  The VPR project is maintained on site, the user manual may be downloaded here: [User Manual](http://www.eecg.utoronto.ca/vtr/docs/VPR_User_Manual_7.0.pdf)
  * Precompiled benchmarks (post-technology mapping) here along with the architecture that tbey were mapped to here.
  * A description of the FPGA architecture language used in VTR may be found [here](VPR_Arch_Language.md)

### Try VTR online using your web browser at [EDA Playground](http://www.edaplayground.com/s/4/420) ###


### Build Notes: ###

  * The complete VTR flow has been tested on 64-bit Linux systems. The flow should work in other platforms (32-bit Linux, Windows with cygwin) but this is untested. Please let us know your experience with building VTR so that we can improve the experience for others.
  * The tools included in the complete VTR package have been tested for compatibility. If you download a different version of those tools, then those versions may not be mutually compatible with the VTR release.
  * For those familiar with the old flow, T-VPack was previously used for packing but we have found that it made more sense for packing to be included in with the placement and routing tool because a flexible packer needs far more awareness of the FPGA architecture than in the simplified models used prior to VPR 6.0.

## Contributors ##

Contributors directly involved with the VTR project can be found [here](Contributors.md).

## How to Cite ##

The following paper may be used as a general citation for VTR:

J. Luu, J. Goeders, M. Wainberg, A. Somerville, T. Yu, K.
Nasartschuk, M. Nasr, S. Wang, T. Liu, N. Ahmed, K. B. Kent, J.
Anderson, J. Rose and V. Betz
"VTR 7.0: Next Generation Architecture and CAD System for FPGAs,"
ACM TRETS, Vol. 7, No. 2, June 2014, pp. 6:1 - 6:30.


[BibTeX format](http://www.eecg.utoronto.ca/vtr/vtr_citation.txt)

## Contact Info ##

Odin II: e-mail ken@unb.ca or jamiespa@muohio.edu

ABC: e-mail alanmi.EECS.Berkeley.edu (Note: Although we use ABC in VTR, the developers of ABC do not currently collaborate with us on the VTR project)

VPR, benchmarks, and architectures: e-mail vpr@eecg.utoronto.ca

Future work - Lots of it!

  * Clock tree architectures
  * Verilog language coverage (eg. parameter support)
  * Faster runtime as circuits keep growing
  * Increased robustness
  * Better error messages and error checking
  * Libraries of standard cores
  * Bus-based routing
  * Improved transistor-level modeling
  * White and black box modelling in logic synthesis
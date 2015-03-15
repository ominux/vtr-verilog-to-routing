# VPR 6.0 Beta Architecture Description Language #

This page provides information on the new FPGA architecture description language used by VPR 6.0. This page is geared towards both new and experienced users of vpr.

New users should consult the conference paper that introduces the language [here](http://www.eecg.utoronto.ca/~jluu/publications/luu_vpr_fpga2011.pdf). This paper describes the motivation behind this new language as well as a short tutorial on how to use the language to describe different complex blocks of an FPGA.

New and experienced users alike should consult the VPR user manual (pending, use examples for now) which acts as a reference detailing every property of the language.

Multiple examples of how this language can be used to describe different types of complex blocks are provided as follows:

## Complete Architecture Description Walkthrough Examples ##

  * Example of a classical soft logic block supported by the CAD tools [here](http://www.eecg.utoronto.ca/vpr/utfal_ex1.html).
  * Example of a configurable memory with different aspect ratios and bus-based registers [here](http://www.eecg.utoronto.ca/vpr/utfal_ex2.html) (CAD tool support pending due to bus-based routing).
  * Example of a fracturable multiplier with bus-based registers [here](http://www.eecg.utoronto.ca/vpr/utfal_ex3.html) (CAD tool support pending due to bus-based routing).

## Architecture Description Examples ##

  * Example of a fracturable multiplier supported by the CAD tools [here](http://www.eecg.utoronto.ca/vpr/utfal_ex5.html)
  * Example of a configurable memory supported by the CAD tools [here](http://www.eecg.utoronto.ca/vpr/utfal_ex6.html)
  * Example of a section of a commercial FPGA logic block. This example uses describes a Xilinx Virtex-6 slice. [here](http://www.eecg.utoronto.ca/vpr/utfal_ex4.html) (CAD tool support pending due to carry-chains).

If you have questions regarding the new architecture file, please e-mail vpr@eecg.utoronto.ca
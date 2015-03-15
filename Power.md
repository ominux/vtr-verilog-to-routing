# VersaPower: Power Estimation for VTR #

## Download ##

The code is now located in the trunk.


## Source Code ##

VersaPower source code is located here:
  * vtr\_flow/vpr/SRC/power/

## Compiling ##

Compile Project:
```

make```

## Run VTR flow with VersaPower ##
```
vtr_flow/scripts/run_vtr_flow.pl <verilog> <architecture> -power -cmos_tech <tech_xml>```

Example:
```

vtr_flow/scripts/run_vtr_flow.pl vtr_flow/benchmarks/verilog/ch_intrinsics.v vtr_flow/arch/power/k6_N10_I33_Fi6_L4_frac1_ff2_45nm.xml -power -cmos_tech vtr_flow/tech/PTM_45nm/45nm.xml ```

## Output ##

Power information will be created in ```
temp/<circuit_name>.power```

## Other Resources ##

Additional Verilog Benchmarks:
```
vtr_flow/benchmarks/verilog```

Additional Arch Files:
```
vtr_flow/arch/power/```
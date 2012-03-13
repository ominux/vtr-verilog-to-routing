/* 
Prepacking: Group together technology-mapped netlist blocks before packing.  This gives hints to the packer on what groups of blocks to keep together during packing.
Primary use 1) "Forced" packs (eg LUT+FF pair)
            2) Carry-chains
*/

#ifndef PREPACK_H
#define PREPACK_H
#include "arch_types.h"
#include "util.h"

t_pack_patterns *alloc_and_load_pack_patterns(OUTP int *num_packing_patterns);

#endif

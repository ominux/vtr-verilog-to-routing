################################   MAKEFILE OPTIONS     ####################################

COMPILER = g++

OPTIMIZATION_LEVEL = -O3
# can be -O0 (no optimization) to -O3 (full optimization), or -Os (optimize space)

#############################################################################################

EXE = neon

CC = $(COMPILER)

LIB_DIR = -L.
LIB = -lneon

SRC_DIR = SRC
OBJ_DIR = OBJ

OBJ = $(patsubst $(SRC_DIR)/%.cxx, $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cxx $(SRC_DIR)/*/*.cxx))
DEP := $(OBJ:.o=.d)


OBJ_DIRS = $(sort $(dir $(OBJ)))
INC_DIRS = -ISRC/util -ISRC/base -ISRC/fabric -ISRC/io -ISRC/analysis

# Silently create target directories as need
$(OBJ_DIRS):
	@ mkdir -p $@



WARN_FLAGS = -Wall -Wpointer-arith -Wcast-qual -D__USE_FIXED_PROTOTYPES__ -ansi -pedantic -Wshadow -Wcast-align -D_POSIX_SOURCE -Wno-write-strings

DEBUG_FLAGS = #-g 
OPT_FLAGS = $(OPTIMIZATION_LEVEL)
#INC_FLAGS = -I../libarchfpga/include

FLAGS = $(WARN_FLAGS) -D EZXML_NOMMAP -D_POSIX_C_SOURCE $(DEBUG_FLAGS)


$(EXE): libneon.a Makefile
	$(CC) $(FLAGS) OBJ/main.o -o $@ $(LIB_DIR) $(LIB)

#create .a library but remove main.o
libneon.a: $(OBJ)
	@ ar rcs $@ $(OBJ)
	@ ar d $@ main.o

# The directory follows a "|" to use an existence check instead of the usual
# timestamp check.  Every write to the directory updates the timestamp thus
# without this, all but the last file written to a directory would appear
# to be out of date.
$(OBJ):	OBJ/%.o:$(SRC_DIR)/%.cxx 
	@#OBJ/%.o:$(SRC_DIR)/%.c | $$(dir $$@D)
	$(CC) $(FLAGS) -MD -MP $(INC_DIRS) -c $< -o $@


#-include $(DEP)


clean:
	rm -f $(EXE) $(OBJ) $(DEP) libneon.a

#for debugging. use by typing make at command line followed by print-<variable> to display the variable
print-%: ; @echo $* is $($*)

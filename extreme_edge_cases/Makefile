OBJS_DIR = .objs

# define all of student executables
EXE=camelCaser
EXE_REF=camelCaser_ref

# tests are defined a little differently for this assignment, see below

# list object file dependencies for each
OBJS_CAMELCASER=$(EXE).o camelCaser_tests.o camelCaser_main.o
OBJS_EXE_REF=$(OBJS_DIR)/camelCaser_ref_tests-release.o
OBJS_REF=ref_files/camelCaser_ref_utils.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations -Wmissing-variable-declarations
CFLAGS_COMMON = $(WARNINGS) -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -DDEBUG

# set up linker
LD = clang
LDFLAGS = -lm

# the string in grep must appear in the hostname, otherwise the Makefile will
# not allow the assignment to compile

IS_VM=$(shell hostname | grep "cs341")
VM_OVERRIDE=$(shell echo $$HOSTNAME)
ifeq ($(IS_VM),)
ifneq ($(VM_OVERRIDE),cs241grader)
$(error This assignment must be compiled on the CS341 VMs)
endif
endif

.PHONY: all
all: release

# build types
.PHONY: release
.PHONY: debug

release: $(EXE) $(EXE_REF)
debug:   clean $(EXE:%=%-debug)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a pair of exe and exe-debug targets for each exe
$(EXE)-debug: $(OBJS_CAMELCASER:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE): $(OBJS_CAMELCASER:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_REF): $(OBJS_EXE_REF)
	$(LD) $^ $(OBJS_REF) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf .objs $(EXE) $(EXE:%=%-debug) $(EXE_REF)

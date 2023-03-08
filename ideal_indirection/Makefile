# use gcc with -fsanitize=thread bc clang on student vms (3.4) has buggy thread
# sanitizer
# this means we also need -ltsan in the link step. libtsan must be installed
# (sudo yum install libtsan on student vms)

OBJS_DIR = .objs

# define all the student executables
MMU_TEST=mmu_test
EXES_STUDENT=$(MMU_TEST)

# list object file dependencies for each
OBJS_MMU_TEST=$(MMU_TEST).o mmu.o tlb.o kernel.o segments.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations -Wmissing-variable-declarations
CFLAGS_COMMON = $(WARNINGS) -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g

# set up linker
LD = clang
LDFLAGS = -lm

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
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)

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
# you will need exe and exe-debug for each exe

$(MMU_TEST): $(OBJS_MMU_TEST:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(MMU_TEST)-debug: $(OBJS_MMU_TEST:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug)

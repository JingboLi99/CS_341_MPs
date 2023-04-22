# see ../../deploy.make.common for most of the instructions.

OBJS_DIR = .objs

# define simulator executables
EXE_MAIN=main
EXE_GTEST=gtest
EXE_TEST=scheduler_test

# list object file dependencies for eac
OBJS_TEST=$(EXE_TEST).o
OBJS_GTEST=$(EXE_GTEST).o
OBJS_MAIN=$(EXE_MAIN).o
OBJS_SCHEDULER=libscheduler.o print_functions.o libpriqueue/libpriqueue.o
OBJS_GTHREAD=gthread.o gtswtch.o
OBJS_TEST_DEP=$(OBJS_SCHEDULER) $(OBJS_TEST)
OBJS_GTEST_DEP=$(OBJS_GTEST) $(OBJS_SCHEDULER) $(OBJS_GTHREAD)
OBJS_ALL=$(OBJS_MAIN) $(OBJS_SCHEDULER) $(OBJS_GTHREAD)

# set up compiler
CC = clang
INCLUDES=-I./includes/
WARNINGS = -Wall -Wextra -Wno-unused -Wno-unused-parameter
CFLAGS_COMMON = -O0 $(INCLUDES) $(WARNINGS) -std=c99 -c -D_GNU_SOURCE
CFLAGS_RELEASE = $(CFLAGS_COMMON) 
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -DDEBUG

# set up linker
LD = clang
PROVIDED_LIBRARIES:=$(shell find libs/ -type f -name '*.a' 2>/dev/null)
PROVIDED_LIBRARIES:=$(PROVIDED_LIBRARIES:libs/lib%.a=%)
LDFLAGS = -Llibs/ $(foreach lib,$(PROVIDED_LIBRARIES),-l$(lib)) -lm

# The string in grep must appear in the hostname, otherwise the Makefile will
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

.PHONY: debug
.PHONY: release

release: $(EXE_MAIN) $(EXE_GTEST) $(EXE_TEST)
debug:   $(EXE_MAIN:%=%-debug) $(EXE_GTEST:%=%-debug) $(EXE_TEST:%=%-debug)

#include dependencies
-include $(OBJS_DIR)/*.d
$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(OBJS_DIR)/libpriqueue

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly

$(OBJS_DIR)/gtswtch-debug.o: gthreads/gtswtch.s | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) gthreads/gtswtch.s -o $@

$(OBJS_DIR)/gtswtch-release.o: gthreads/gtswtch.s | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) gthreads/gtswtch.s -o $@

$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

$(OBJS_DIR)/gthread-debug.o: gthreads/gthreads.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) -DLOG=\"gthread.log\" $< -o $@

$(OBJS_DIR)/gthread-release.o: gthreads/gthreads.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) -DLOG=\"gthread.log\" $< -o $@

# exes
# you will need exe and exe-debug
$(EXE_MAIN): $(OBJS_ALL:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_MAIN)-debug: $(OBJS_ALL:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_GTEST): $(OBJS_GTEST_DEP:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_GTEST)-debug: $(OBJS_GTEST_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TEST): $(OBJS_TEST_DEP:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_TEST)-debug: $(OBJS_TEST_DEP:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

.PHONY : clean
clean:
	rm -rf .objs $(EXE_MAIN) $(EXE_MAIN:%=%-debug) $(EXE_GTEST) $(EXE_GTEST:%=%-debug) $(EXE_TEST) $(EXE_TEST:%=%-debug) gthread.log

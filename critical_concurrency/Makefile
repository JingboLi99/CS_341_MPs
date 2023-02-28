# use gcc with -fsanitize=thread bc clang on student vms (3.4) has buggy thread
# sanitizer
# this means we also need -ltsan in the link step. libtsan must be installed
# (sudo yum install libtsan on student vms)

OBJS_DIR = .objs

# define all the student executables
EXE_RENDE=rendezvous
EXE_SEM=semamore_test
EXE_QUEUE=queue_test
EXE_BAR=barrier_test
EXES_STUDENT=$(EXE_BAR) $(EXE_SEM) $(EXE_RENDE) $(EXE_QUEUE)

# list object file dependencies for each
OBJS_RENDE=$(EXE_RENDE).o semamore.o
OBJS_SEM=$(EXE_SEM).o semamore.o
OBJS_QUEUE=$(EXE_QUEUE).o queue.o
OBJS_BAR=$(EXE_BAR).o barrier.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations -Wmissing-variable-declarations
CFLAGS_COMMON = $(WARNINGS) -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -DDEBUG

# tsan needs some funky flags
CFLAGS_TSAN_DEBUG    = $(CFLAGS_DEBUG)
CFLAGS_TSAN_DEBUG    += -fsanitize=thread -DSANITIZE_THREADS -fPIC

CFLAGS_TSAN = $(CFLAGS_TSAN_DEBUG) -UDEBUG

# set up linker
LD = clang
LDFLAGS = -lrt -lpthread -fPIC -lm
LDFLAGS_TSAN = -ltsan $(LDFLAGS)

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
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release
.PHONY: tsan
.PHONY: debug-tsan

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)
tsan:    clean $(EXES_STUDENT:%=%-tsan)
debug-tsan:    clean $(EXES_STUDENT:%=%-debug-tsan)

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

$(OBJS_DIR)/%-debug-tsan.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_TSAN_DEBUG) $< -o $@

$(OBJS_DIR)/%-tsan.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_TSAN) $< -o $@

# exes
# you will need a quadruple of exe, exe-debug, exe-debug-tsan and exe-tsan for
# each exe (other than provided exes)

# arguments:
#   $1 - name of target without "EXE_" prefix
# returns makefile source to generate all required targets
# To use, "call" this definition with an argument and then "eval" the result.
# See below for examples.
define target_template
ifeq ($1,)
$$(error target_template expected an argument!)
endif
$$(EXE_$1): $$(OBJS_$1:%.o=$$(OBJS_DIR)/%-release.o)
	$$(LD) $$^ $$(LDFLAGS) -o $$@

$$(EXE_$1)-debug: $$(OBJS_$1:%.o=$$(OBJS_DIR)/%-debug.o)
	$$(LD) $$^ $$(LDFLAGS) -o $$@

$$(EXE_$1)-debug-tsan: $$(OBJS_$1:%.o=$$(OBJS_DIR)/%-debug-tsan.o)
	$$(LD) $$^ $$(LDFLAGS_TSAN) -o $$@

$$(EXE_$1)-tsan: $$(OBJS_$1:%.o=$$(OBJS_DIR)/%-tsan.o)
	$$(LD) $$^ $$(LDFLAGS_TSAN) -o $$@
endef

$(eval $(call target_template,BAR))
$(eval $(call target_template,RENDE))
$(eval $(call target_template,QUEUE))
$(eval $(call target_template,SEM))

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT)\
		$(EXES_STUDENT:%=%-debug)\
		$(EXES_STUDENT:%=%-debug-tsan)\
		$(EXES_STUDENT:%=%-tsan)

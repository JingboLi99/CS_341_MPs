OBJS_DIR = .objs

# define all of student executables
EXE1=dd
EXES_STUDENT=$(EXE1)

# list object file dependencies for each
OBJS=format.o dd.o

# set up compiler
CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter -Wmissing-declarations -Wmissing-variable-declarations
CFLAGS_COMMON = $(WARNINGS) -std=c99 -c -MMD -MP -D_GNU_SOURCE
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g

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
# you will need a pair of exe and exe-debug targets for each exe
$(EXE1)-debug: $(OBJS:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE1): $(OBJS:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@


# test data
test-data:
	@echo "Generating test data..."
	@mkdir -p test_files

	# generate some random data of size 512 KB, 1 MB, 16 MB, 128 MB, 512 MB
	@dd if=/dev/urandom of=test_files/512KB.dat bs=512K count=1
	@dd if=/dev/urandom of=test_files/1MB.dat bs=1M count=1
	@dd if=/dev/urandom of=test_files/16MB.dat bs=1M count=16
	@dd if=/dev/urandom of=test_files/128MB.dat bs=1M count=128
	@dd if=/dev/urandom of=test_files/512MB.dat bs=1M count=512

	@# fetch some cool space images
	@wget -O test_files/earth.jpg images-assets.nasa.gov/image/PIA18033/PIA18033~orig.jpg
	@wget -O test_files/moon.jpg images-assets.nasa.gov/image/as16-113-18339/as16-113-18339~orig.jpg
	@wget -O test_files/orion.mp4 images-assets.nasa.gov/video/NHQ_2014_1107_TWAN/NHQ_2014_1107_TWAN~orig.mp4

	@# fetch romeo and juliet
	@wget -O test_files/romeo_and_juliet.txt http://www.textfiles.com/etext/AUTHORS/SHAKESPEARE/shakespeare-romeo-48.txt

.PHONY: clean
clean:
	rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-debug) test_files

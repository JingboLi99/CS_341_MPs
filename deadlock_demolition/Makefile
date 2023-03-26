CC = gcc

OBJS_DIR = .objs
EXE_DRM = libdrm_tester
EXES_STUDENT = $(EXE_DRM)
# list object file dependencies for each
OBJS_DRM=libdrm.o libdrm_tester.o

INC = -I./includes/
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
CFLAGS_COMMON = $(WARNINGS) $(INC) -std=c99 -c -MMD -MP -D_GNU_SOURCE -DTHREAD_SAFE -pthread
CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2
CFLAGS_DEBUG = $(CFLAGS_COMMON) -O0 -g -DDEBUG
CFLAGS_TSAN = $(CFLAGS_RELEASE) -g

# set up linker
PROVIDED_LIBRARIES:=$(shell find libs/ -type f -name '*.a' 2>/dev/null)
PROVIDED_LIBRARIES:=$(PROVIDED_LIBRARIES:libs/lib%.a=%)
LD=$(CC)
LDFLAGS = -pthread -Llibs/ $(foreach lib,$(PROVIDED_LIBRARIES),-l$(lib)) -lm
LDFLAGS_TSAN = $(LDFLAGS) -ltsan

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: release
.PHONY: tsan
.PHONY: debug

release: $(EXES_STUDENT)
tsan:    clean $(EXES_STUDENT:%=%-tsan)
debug: $(EXES_STUDENT:%=%-debug)

-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

$(OBJS_DIR)/%-tsan.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_TSAN) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	@mkdir -p $(basename $@)
	$(CC) $(CFLAGS_DEBUG) $< -o $@
# exes
# you will need a triple of exe and exe-debug and exe-tsan for each exe (other
# than provided exes)
$(EXE_DRM): $(OBJS_DRM:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_DRM)-debug: $(OBJS_DRM:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_DRM)-debug-tsan: $(OBJS_DRM:%.o=$(OBJS_DIR)/%-debug-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

$(EXE_DRM)-tsan: $(OBJS_DRM:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT)\
		$(EXES_STUDENT:%=%-tsan)\
		$(EXES_STUDENT:%=%-debug)\
		$(EXES_STUDENT:%=%-debug-tsan)

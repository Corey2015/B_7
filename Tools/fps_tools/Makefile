# ------------------------------------------------------------------------------
# Output Files
# ------------------------------------------------------------------------------

LIB_NAME = libfps.a
EXE_NAME = $(shell basename $(CURDIR))


# ------------------------------------------------------------------------------
# Source Files
# ------------------------------------------------------------------------------

APP_SRCS = $(shell echo analyzer_cli/*.c)

LIB_SRCS = $(shell echo library/*.c) \
		   $(shell echo library/linux/*.c)


# ------------------------------------------------------------------------------
# Toolchain Settings
# ------------------------------------------------------------------------------

CROSS_TOOLCHAIN = arm-linux-
CC              = $(CROSS_TOOLCHAIN)gcc
AR              = $(CORSS_TOOLCHAIN)ar

DEBUG_FLAGS     = -O0 -g -D__DEBUG__
RELEASE_FLAGS   = -O2
CC_FLAGS        = -Wall -Wno-psabi -Iinclude -static -D__LINUX__ -D__F747B__

LIB_CC_FLAGS    = $(CC_FLAGS) -Ilibrary $(DEBUG_FLAGS)
LIB_LD_FLAGS    =
LIB_AR_FLAGS    = rcs

APP_CC_FLAGS    = $(CC_FLAGS) -Ilibrary $(DEBUG_FLAGS)
APP_LD_FLAGS    = -lm


# ------------------------------------------------------------------------------
# Compile Executable
# ------------------------------------------------------------------------------

.PHONY: all
all: $(EXE_NAME)
	@mv $^ $^.`date +%Y-%b-%d_%H-%M-%S`

$(EXE_NAME): $(APP_SRCS) $(LIB_NAME)
	$(CC) $(APP_CC_FLAGS) -o $@ $^ $(APP_LD_FLAGS)


# ------------------------------------------------------------------------------
# Compile Library
# ------------------------------------------------------------------------------

LIB_OBJS = $(LIB_SRCS:.c=.o)

.PHONY: lib
lib: $(LIB_NAME)

$(LIB_NAME): $(LIB_OBJS)
	$(AR) $(LIB_AR_FLAGS) $@ $^

%.o: %.c
	$(CC) $(LIB_CC_FLAGS) -c -o $@ $<


# ------------------------------------------------------------------------------
# Clean Up
# ------------------------------------------------------------------------------

.PHONY: clean
clean:
	-@echo -n "Cleanning... "
	-@$(RM) $(LIB_NAME)
	-@$(RM) $(EXE_NAME).201*
	-@$(RM) $(LIB_OBJS)
	-@$(foreach i, $(shell ls -d */ */*/), $(RM) $(i)/*~ $(i)/.*~)
	-@$(RM) *~ .*~
	-@echo "Done!"


# ------------------------------------------------------------------------------
# Download
# ------------------------------------------------------------------------------

PROGRAM_NAME  = $(shell ls -t $(EXE_NAME).*-*-*_*-*-* | head -1)
TARGET_FOLDER = /data/dolfa
LINK_NAME     = tool

.PHONY: push
push:
	-@adb remount
	-@adb push $(PROGRAM_NAME) $(TARGET_FOLDER)
	-@adb shell rm -rf $(TARGET_FOLDER)/$(LINK_NAME)
	-@adb shell ln -fs $(TARGET_FOLDER)/$(PROGRAM_NAME) $(TARGET_FOLDER)/$(LINK_NAME)

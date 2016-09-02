# Configuration

BUILD ?= ./build

# optimizations are *required*, due to tail/sibling calls
# "-O1 -foptimize-sibling-calls" works, but -O2 is shorter
EXTRA_CFLAGS  ?= -Wall -Wextra -pedantic -std=c99 -O2
DEP_FILES     != [ -d $(BUILD)/deps ] && find $(BUILD)/deps -name \*.d
REAc_SOURCE   != find reaCtive -name \*.c
REAc_OBJ      := $(REAc_SOURCE:%.c=$(BUILD)/%.o)

all: $(BUILD)/proofofconcept.exe

.PHONY: all clean print

# Build Rules

$(BUILD)/%.o: %.c
	mkdir -p $(BUILD)/$(*D) $(BUILD)/deps/$(*D)
	$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(BUILD)/deps/$*.d

%.exe:
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^

# Utility

clean:
	-rm -r $(BUILD)

print:
	@echo BUILD=$(BUILD)
	@echo EXTRA_CFLAGS=$(EXTRA_CFLAGS)
	@echo DEP_FILES=$(DEP_FILES)
	@echo REAc_SOURCE=$(REAc_SOURCE)
	@echo REAc_OBJ=$(REAc_OBJ)
	@echo
	@echo CC=$(CC)
	@echo LD=$(LD)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)

test: $(BUILD)/proofofconcept.exe
	$(BUILD)/proofofconcept.exe

# Dependencies

-include $(DEP_FILES)

$(BUILD)/proofofconcept.exe: $(BUILD)/proofofconcept.o $(REAc_OBJ)

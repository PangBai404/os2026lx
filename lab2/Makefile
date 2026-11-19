CC      := gcc
CFLAGS  := -Wall -Wextra -O2
BUILD   := build
HARNESS := harness/2_harness.c harness/impl.h

all: $(BUILD)/2_harness

$(BUILD)/2_harness: $(HARNESS) solution/impl.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Iharness harness/2_harness.c solution/impl.c -o $@

$(BUILD)/2_harness_student: $(HARNESS) student/impl.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Iharness harness/2_harness.c student/impl.c -o $@

test: $(BUILD)/2_harness
	python3 tests/check.py $(BUILD)/2_harness

test-student: $(BUILD)/2_harness_student
	python3 tests/check.py $(BUILD)/2_harness_student

audit:
	python3 tests/check.py --audit student/impl.c

clean:
	rm -rf $(BUILD) f1

.PHONY: all test test-student audit clean

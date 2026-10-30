# Host-side tests for keymap code that doesn't depend on QMK.
# Each test_<name>.c is linked against $(KEYMAP)/<name>.c.

KEYMAP := ../keyboards/cosmos/anex/keymaps/via
BUILD  := build
CFLAGS := -std=c11 -Wall -Wextra -Werror -O1 -g -I$(KEYMAP) -I.

TESTS := $(patsubst test_%.c,%,$(wildcard test_*.c))

.PHONY: all clean
all: $(addprefix run-,$(TESTS))

run-%: $(BUILD)/test_%
	./$<

$(BUILD)/test_%: test_%.c $(KEYMAP)/%.c $(KEYMAP)/%.h check.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $< $(KEYMAP)/$*.c

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

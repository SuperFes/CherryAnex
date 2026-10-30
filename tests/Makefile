# Host-side tests for keymap code that doesn't depend on QMK.
# Each test_<name>.c is linked against $(KEYMAP)/<name>.c, plus any modules
# listed in DEPS_<name>.

KEYMAP := ../keyboards/cosmos/anex/keymaps/via
BUILD  := build
CFLAGS := -std=c11 -Wall -Wextra -Werror -O1 -g -I$(KEYMAP) -I.

TESTS := $(patsubst test_%.c,%,$(wildcard test_*.c))

DEPS_saver := canvas

.PHONY: all clean
all: $(addprefix run-,$(TESTS))

run-%: $(BUILD)/test_%
	./$<

.SECONDEXPANSION:
$(BUILD)/test_%: test_%.c $(KEYMAP)/%.c $(KEYMAP)/%.h $$(addprefix $(KEYMAP)/,$$(addsuffix .c,$$(DEPS_$$*))) check.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $< $(KEYMAP)/$*.c $(addprefix $(KEYMAP)/,$(addsuffix .c,$(DEPS_$*)))

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

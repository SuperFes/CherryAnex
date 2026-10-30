# Host-side tests for keymap code that doesn't depend on QMK.

KEYMAP := ../keyboards/cosmos/anex/keymaps/via
BUILD  := build
CFLAGS := -std=c11 -Wall -Wextra -Werror -O1 -g -I$(KEYMAP) -I.

TESTS := status

.PHONY: all clean
all: $(addprefix run-,$(TESTS))

run-%: $(BUILD)/test_%
	./$<

$(BUILD)/test_status: test_status.c $(KEYMAP)/status.c $(KEYMAP)/status.h check.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ test_status.c $(KEYMAP)/status.c

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)

# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright (c) 2026 Fernando Lugo <lugo.fernando@gmail.com>

.PHONY: flash

FLASH_SCRIPT ?= src/$(BOARD_PATH)/flash-rp2350.sh

flash: $(BUILD_DIR)/sc.elf
	$(Q)$(FLASH_SCRIPT) $<

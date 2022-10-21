
# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2021, Fernando Lugo <lugo.fernando@gmail.com>

.PHONY: qemu

qemu: $(BUILD_DIR)/sc.bin
	$(Q)qemu-system-aarch64 -M virt,secure=on,virtualization=on -cpu max -m 3072 -smp 2 -nographic -semihosting -s -kernel $<

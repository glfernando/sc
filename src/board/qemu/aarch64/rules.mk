
# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2021, Fernando Lugo <lugo.fernando@gmail.com>

.PHONY: qemu

qemu: $(BUILD_DIR)/sc.bin
	$(Q)qemu-system-aarch64 -M virt,secure=on,virtualization=on -cpu cortex-a53 -m 512 -smp 1 -nographic -semihosting -kernel $<

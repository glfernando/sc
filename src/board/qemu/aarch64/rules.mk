
# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2021, Fernando Lugo <lugo.fernando@gmail.com>

.PHONY: qemu

ifeq ($(CONFIG_AARCH64_MTE), y)
QEMU_MACHINE = virt,secure=on,virtualization=on,mte=on
else
QEMU_MACHINE = virt,secure=on,virtualization=on
endif

qemu: $(BUILD_DIR)/sc.bin
	$(Q)qemu-system-aarch64 -M $(QEMU_MACHINE) -cpu max -m 3072 -smp 2 -nographic -semihosting -s -kernel $<


src-y += cpu.cppm

ifneq ($(filter armv6m armv8m,$(CPU)),)
src-y += armv6m.cppm
endif

ifeq ($(CPU), armv8)
src-y += armv8.cppm
endif

src-$(CONFIG_CPU_SOC_DUMMY) += soc.cppm

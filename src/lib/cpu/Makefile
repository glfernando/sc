
src-y += cpu.cppm

ifeq ($(CPU), armv6m)
src-y += armv6m.cppm
endif

ifeq ($(CPU), armv8)
src-y += armv8.cppm
endif

src-$(CONFIG_CPU_SOC_DUMMY) += soc.cppm

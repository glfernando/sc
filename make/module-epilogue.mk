# SPDX-License-Identifier: BSD-3-Clause
#
# (C) Copyright 2020, Fernando Lugo <lugo.fernando@gmail.com>

## remove me from include list
MAKEFILE_LIST := $(lastword $(filter-out $(lastword $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))

## add current directory prefix
src-y := $(addprefix $(MODULE_PATH)/,$(src-y))
c_srcs += $(filter %.c, $(src-y))
asm_srcs += $(filter %.S, $(src-y))
cpp_srcs += $(filter %.cpp, $(src-y))
mod_srcs += $(filter %.cppm, $(src-y))
dir-y := $(filter %/, $(src-y))

define create_module_flags
  out_file := $$(addprefix $$(BUILD_DIR)/,$$(dir $(1)))
  out_dir := $$(patsubst %/,%,$$(dir $$(out_file)))
  out_dir_var_prefix := $$(subst /,_,$$(out_dir))

  CPPFLAGS_$$(out_dir_var_prefix) := $$(MODULE_CPPFLAGS)
  CFLAGS_$$(out_dir_var_prefix) := $$(MODULE_CFLAGS)
  CXXFLAGS_$$(out_dir_var_prefix) := $$(MODULE_CXXFLAGS)
endef

$(foreach file,$(filter-out $(diry),$(src-y)),$(eval $(call create_module_flags,$(file))))

# include all other directores
module_dirs += $(addsuffix Makefile,$(dir-y))
$(foreach file,$(module_dirs),$(eval $(call include_module,$(file))))

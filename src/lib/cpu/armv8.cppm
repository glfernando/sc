

export module lib.cpu.arch;

export namespace lib::cpu {

void disable_irq() {
    asm volatile("msr daifset, #3");
}

void enable_irq() {
    asm volatile("msr daifclr, #3");
}

long save_and_disable_irq() {
    long flags;
    // clang-format off
    asm volatile(R"(
        mrs %0, daif
        msr daifset, #3
    )" : "=r"(flags));
    // clang-format on
    return flags;
}

void restore_irq(long flags) {
    asm volatile("msr daif, %0" ::"r"(flags));
}

}  // namespace lib::cpu

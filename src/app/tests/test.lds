SECTIONS {
    .tests : {
        __tests_start = .;
        KEEP(*(SORT_BY_NAME(.tests*)))
        __tests_end = .;
    }
}
INSERT AFTER .rodata;

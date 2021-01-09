/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (C) 2021 Fernando Lugo <lugo.fernando@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unwind.h>

namespace __cxxabiv1 {

// type info structs
struct __class_type_info {
    virtual void foo() {}
} cti;
struct __fundamental_type_info {
    virtual void foo() {}
} fti;
struct __pointer_type_info {
    virtual void foo() {}
} pti;

struct __si_class_type_info {
    virtual void foo() {}
} scti;

}  // namespace __cxxabiv1

namespace std {
// TODO: move terminate to a better place
[[noreturn]] void terminate() noexcept {
    printf("terminate\n");
    while (1) {}
}
}  // namespace std

static _Unwind_Exception* curr_excep;

extern "C" void* __cxa_begin_catch(void* exceptionObject) {
    _Unwind_Exception* ue = static_cast<_Unwind_Exception*>(exceptionObject);
    curr_excep = ue;
    return ue + 1;
}

extern "C" void __cxa_end_catch() {
    curr_excep = nullptr;
}

extern "C" void __cxa_rethrow() {
    if (curr_excep)
        _Unwind_RaiseException(curr_excep);
    std::terminate();
}

extern "C" int __cxa_atexit(void (*)(void*), void*, void*) {
    return 0;
}

extern "C" void __cxa_pure_virtual() {
    while (true) {}
}

struct __cxa_exception {
    struct type_info* exceptionType;
    _Unwind_Exception unwindHeader;
};

extern "C" void* __cxa_allocate_exception(size_t size) {
    size_t total = sizeof(__cxa_exception) + size;
    void* ptr = malloc(total);
    memset(ptr, 0, size);
    ptr = ((__cxa_exception*)ptr + 1);
    return ptr;
}

extern "C" void __cxa_free_exception(void* thrown_exception) {
    __cxa_exception* header = ((__cxa_exception*)thrown_exception - 1);
    free(header);
}

static const uint64_t EXCEPTION_CLASS_CLANG = 0x434C4E47432B2B00;

extern "C" void __cxa_throw(void* thrown_exception, struct type_info* tinfo, void (*dest)(void*)) {
    __cxa_exception* header = ((__cxa_exception*)thrown_exception - 1);
    header->exceptionType = tinfo;
    header->unwindHeader.exception_class = EXCEPTION_CLASS_CLANG;
    _Unwind_Reason_Code code = _Unwind_RaiseException(&header->unwindHeader);

    // __cxa_throw never returns
    printf("no valid catch. Unwind_RaiseException reason code %x\n", code);
    std::terminate();
}

enum dw_enc : uint8_t {
    DW_EH_PE_ptr = 0x00,
    DW_EH_PE_uleb128 = 0x01,
    DW_EH_PE_udata2 = 0x02,
    DW_EH_PE_udata4 = 0x03,
    DW_EH_PE_udata8 = 0x04,
    DW_EH_PE_signed = 0x08,
    DW_EH_PE_sleb128 = 0x09,
    DW_EH_PE_sdata2 = 0x0A,
    DW_EH_PE_sdata4 = 0x0B,
    DW_EH_PE_sdata8 = 0x0C,
    DW_EH_PE_absptr = 0x00,
    DW_EH_PE_pcrel = 0x10,
    DW_EH_PE_textrel = 0x20,
    DW_EH_PE_datarel = 0x30,
    DW_EH_PE_funcrel = 0x40,
    DW_EH_PE_aligned = 0x50,
    DW_EH_PE_indirect = 0x80,
    DW_EH_PE_omit = 0xFF
};

static uintptr_t readULEB128(const uint8_t*& p) {
    uintptr_t result = 0;
    unsigned shift = 0;
    uint8_t byte;

    do {
        byte = *p++;
        result |= static_cast<uintptr_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    return result;
}

static intptr_t readSLEB128(const uint8_t*& p) {
    uintptr_t result = 0;
    unsigned shift = 0;
    uint8_t byte;

    do {
        byte = *p++;
        result |= static_cast<uintptr_t>(byte & 0x7F) << shift;
        shift += 7;
    } while (byte & 0x80);

    if ((shift < (sizeof(result) << 3)) && (byte & 0x40))
        result |= static_cast<uintptr_t>(~0) << shift;

    return static_cast<intptr_t>(result);
}

static uintptr_t read_enc_ptr(const uint8_t*& p, dw_enc enc) {
    if (enc == DW_EH_PE_omit)
        return 0;

    uintptr_t result = 0;
    switch (enc & 0xf) {
    case DW_EH_PE_absptr:
        break;
    case DW_EH_PE_uleb128:
        result = readULEB128(p);
        break;
    case DW_EH_PE_sleb128:
        result = static_cast<uintptr_t>(readSLEB128(p));
        break;
    case DW_EH_PE_udata4:
        result = *reinterpret_cast<const uint32_t*>(p);
        p += 4;
        break;
    case DW_EH_PE_sdata8:
        result = *reinterpret_cast<const int64_t*>(p);
        p += 8;
        break;
    default:
        printf("unsupported encoding = %x\n", enc);
    }

    // then add relative offset
    switch (enc & 0x70) {
    case DW_EH_PE_absptr:
        // do nothing
        break;
    case DW_EH_PE_pcrel:
        if (result)
            result += (uintptr_t)(p - 8);
        break;
    case DW_EH_PE_textrel:
    case DW_EH_PE_datarel:
    case DW_EH_PE_funcrel:
    case DW_EH_PE_aligned:
    default:
        // not supported
        printf("not supported enconding\n");
        std::terminate();
    }
    // then apply indirection
    if (result && (enc & DW_EH_PE_indirect))
        result = *((uintptr_t*)result);
    return result;
}

struct call_site {
    uintptr_t start;
    uintptr_t len;
    uintptr_t lp;
    uintptr_t action_offset;
};

struct lsda_action {
    intptr_t ttype_index;
    intptr_t next_offset;
    const uint8_t* base;
    operator bool() const { return base != nullptr; }
    lsda_action get_next() {
        lsda_action la{};

        if (!next_offset)
            return la;

        const uint8_t* ptr = base + next_offset;

        la.ttype_index = readSLEB128(ptr);
        la.base = ptr;
        la.next_offset = readSLEB128(ptr);

        return la;
    }
};

class Lsda {
 public:
    Lsda(const uintptr_t ptr) : curr(reinterpret_cast<const uint8_t*>(ptr)) {
        lp_start_enc = static_cast<dw_enc>(*curr++);
        lp_start = read_enc_ptr(curr, lp_start_enc);
        ttype_enc = static_cast<dw_enc>(*curr++);
        ttype_base_offset = ttype_enc == DW_EH_PE_omit ? 0 : readULEB128(curr);
        ttype_base = ttype_base_offset ? curr + ttype_base_offset : nullptr;

        call_site_enc = static_cast<dw_enc>(*curr++);
        call_site_len = readULEB128(curr);
        call_site_curr = call_site_start = curr;
        action_table_start = call_site_end = call_site_start + call_site_len;
    }

    call_site read_call_site() {
        call_site cs{};
        if (call_site_curr < call_site_end) {
            cs.start = read_enc_ptr(call_site_curr, call_site_enc);
            cs.len = read_enc_ptr(call_site_curr, call_site_enc);
            cs.lp = read_enc_ptr(call_site_curr, call_site_enc);
            cs.action_offset = readULEB128(call_site_curr);
        }

        return cs;
    }

    void call_site_reset() { call_site_curr = call_site_start; }

    lsda_action get_action(uintptr_t offset) {
        lsda_action la{};

        if (!offset)
            return la;

        const uint8_t* ptr = action_table_start + offset - 1;

        la.ttype_index = readSLEB128(ptr);
        la.base = ptr;  // save this pointer which we will need for geting next action
        la.next_offset = readSLEB128(ptr);
        return la;
    }

    const void* get_type_info(intptr_t ttype_index) {
        switch (ttype_enc & 0x0F) {
        case DW_EH_PE_absptr:
            ttype_index *= sizeof(uintptr_t);
            break;
        case DW_EH_PE_udata2:
        case DW_EH_PE_sdata2:
            ttype_index *= 2;
            break;
        case DW_EH_PE_udata4:
        case DW_EH_PE_sdata4:
            ttype_index *= 4;
            break;
        case DW_EH_PE_udata8:
        case DW_EH_PE_sdata8:
            ttype_index *= 8;
            break;
        default:
            // this should never happen. It indicates corrupted eh_table.
            printf("invalid coding\n");
            std::terminate();
        }
        const uint8_t* ptr = ttype_base - ttype_index;
        return reinterpret_cast<void*>(read_enc_ptr(ptr, ttype_enc));
    }

 private:
    const uint8_t* curr;
    dw_enc lp_start_enc;
    uintptr_t lp_start;
    dw_enc ttype_enc;
    uintptr_t ttype_base_offset;
    dw_enc call_site_enc;
    uintptr_t call_site_len;
    const uint8_t* call_site_start;
    const uint8_t* call_site_curr;
    const uint8_t* call_site_end;
    const uint8_t* action_table_start;
    const uint8_t* ttype_base;
};

static _Unwind_Reason_Code install_pad(_Unwind_Exception* unwind_exception,
                                       _Unwind_Context* context, int type_index,
                                       uintptr_t ip_addr) {
    _Unwind_SetGR(context, __builtin_eh_return_data_regno(0),
                  reinterpret_cast<uintptr_t>(unwind_exception));
    _Unwind_SetGR(context, __builtin_eh_return_data_regno(1), type_index);
    _Unwind_SetIP(context, ip_addr);
    return _URC_INSTALL_CONTEXT;
}

extern "C" _Unwind_Reason_Code __gxx_personality_v0(int version, _Unwind_Action actions,
                                                    uint64_t exceptionClass,
                                                    _Unwind_Exception* unwind_exception,
                                                    _Unwind_Context* context) {
    uintptr_t ip = _Unwind_GetIP(context) - 1;

    Lsda lsda(_Unwind_GetLanguageSpecificData(context));

    for (call_site cs = lsda.read_call_site(); cs.len; cs = lsda.read_call_site()) {
        // if not landing pad skip it
        if (not cs.lp)
            continue;

        uintptr_t func_start = _Unwind_GetRegionStart(context);
        uintptr_t try_start = func_start + cs.start;
        uintptr_t try_end = func_start + cs.start + cs.len;

        // Check if this is the correct LP for the current try block
        if (ip < try_start)
            continue;
        if (ip > try_end)
            continue;

        if (!cs.action_offset) {
            if (actions == _UA_CLEANUP_PHASE)
                return install_pad(unwind_exception, context, 0, func_start + cs.lp);
            continue;
        }

        uintptr_t offset = cs.action_offset;
        for (lsda_action action = lsda.get_action(offset); action; action = action.get_next()) {
            if (action.ttype_index) {
                auto ti =
                    static_cast<const struct type_info*>(lsda.get_type_info(action.ttype_index));

                __cxa_exception* header = (__cxa_exception*)(unwind_exception + 1) - 1;
                if (ti && header->exceptionType != ti)
                    continue;

                // if we are in search phase tell we can handle the exception
                if (actions & _UA_SEARCH_PHASE)
                    return _URC_HANDLER_FOUND;

                // assume cleanup phase
                return install_pad(unwind_exception, context, action.ttype_index,
                                   func_start + cs.lp);
            } else if (actions & _UA_CLEANUP_PHASE) {  // ttype_index == 0, so it is a cleanup LP
                return install_pad(unwind_exception, context, action.ttype_index,
                                   func_start + cs.lp);
            }
        }
    }
    return _URC_CONTINUE_UNWIND;
}

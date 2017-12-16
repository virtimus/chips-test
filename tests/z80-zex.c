//------------------------------------------------------------------------------
//  z80-zex.c
//
//  Runs Frank Cringle's zexdoc and zexall test through the Z80 emu. Provide
//  a minimal CP/M environment to make these work.
//------------------------------------------------------------------------------
#define CHIPS_IMPL
#include "chips/z80.h"
#define SOKOL_IMPL
#include "sokol_time.h"
#include "roms/zex-dump.h"
#include <stdio.h>

enum {
    mem_size = 1<<16,
    mem_mask = mem_size-1,
    output_size = 1<<16
};

static uint8_t mem[mem_size];
static int out_pos = 0;
static char output[output_size];

static void put_char(char c) {
    if (out_pos < output_size) {
        output[out_pos++] = c;
    }
    putchar(c);
}

/* Z80 tick callback */
static uint64_t tick(int num, uint64_t pins) {
    if (pins & Z80_MREQ) {
        if (pins & Z80_RD) {
            Z80_SET_DATA(pins, mem[Z80_ADDR(pins)]);
        }
        else if (pins & Z80_WR) {
            mem[Z80_ADDR(pins)] = Z80_DATA(pins);
        }
    }
    return pins;
}

/* emulate character and string output CP/M system calls */
static bool cpm_bdos(z80_t* cpu) {
    bool retval = true;
    if (2 == cpu->C) {
        // output character in register E
        put_char(cpu->E);
    }
    else if (9 == cpu->C) {
        // output $-terminated string pointed to by register DE */
        uint8_t c;
        uint16_t addr = cpu->DE;
        while ((c = mem[addr++ & mem_mask]) != '$') {
            put_char(c);
        }
    }
    else {
        printf("Unhandled CP/M system call: %d\n", cpu->C);
        retval = false;
    }
    // emulate a RET
    uint8_t z = mem[cpu->SP++];
    uint8_t w = mem[cpu->SP++];
    cpu->PC = cpu->WZ = (w<<8)|z;
    return retval;
}

/* run CPU through the configured test (ZEXDOC or ZEXALL) */
static bool run_test(z80_t* cpu, const char* name) {
    bool running = true;
    uint64_t ticks = 0;
    uint64_t start_time = stm_now();
    while (running) {
        /* run for a lot of ticks or until HALT is encountered */
        ticks += z80_exec(cpu, (1<<30));
        /* check for BDOS call */
        if (5 == cpu->PC) {
            if (!cpm_bdos(cpu)) {
                running = false;
            }
        }
        else if (0 == cpu->PC) {
            running = false;
        }
        cpu->PINS &= ~Z80_HALT;
    }
    double dur = stm_sec(stm_since(start_time));
    printf("\n%s: %llu cycles in %.3fsecs (%.2f MHz)\n", name, ticks, dur, (ticks/dur)/1000000.0);

    /* check if an error occurred */
    if (output_size > 0) {
        output[output_size-1] = 0;
        if (strstr((const char*)output, "ERROR")) {
            return false;
        }
        else {
            printf("\n\n ALL %s TESTS PASSED!\n", name);
        }
    }
    return true;
}

/* run the ZEXDOC test */
static bool zexdoc() {
    memset(output, 0, sizeof(output));
    memset(mem, 0, sizeof(mem));
    memcpy(&mem[0x0100], dump_zexdoc, sizeof(dump_zexdoc));
    z80_t cpu;
    z80_init(&cpu, tick);
    /* break out of z80_exec when HALT instruction is encountered */
    cpu.break_mask |= Z80_HALT;
    cpu.SP = 0xF000;
    cpu.PC = 0x0100;
    /* patch a HALT instruction at address 0x0000 and 0x0005 */
    mem[0] = 0x76;
    mem[5] = 0x76;
    return run_test(&cpu, "ZEXDOC");
}

/* run the ZEXALL test */
static bool zexall() {
    memset(output, 0, sizeof(output));
    memset(mem, 0, sizeof(mem));
    memcpy(&mem[0x0100], dump_zexall, sizeof(dump_zexall));
    z80_t cpu;
    z80_init(&cpu, tick);
    /* break out of z80_exec when HALT instruction is encountered */
    cpu.break_mask |= Z80_HALT;
    cpu.SP = 0xF000;
    cpu.PC = 0x0100;
    /* patch a HALT instruction at address 0x0000 and 0x0005 */
    mem[0] = 0x76;
    mem[5] = 0x76;
    return run_test(&cpu, "ZEXALL");
}

int main() {
    stm_setup();
    if (!zexdoc()) {
        return 10;
    }
    if (!zexall()) {
        return 10;
    }
    return 0;
}
#include "gb.h"
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif
#include <windows.h>
#else
#include <sys/time.h>
#endif

static const unsigned GB_TAC_TRIGGER_BITS[] = {512, 8, 32, 128};

#define IR_DECAY 31500
#define IR_THRESHOLD 19900
#define IR_MAX IR_THRESHOLD * 2 + IR_DECAY

static void advance_tima_state_machine(GB_gameboy_t *gb)
{
    if (gb->tima_reload_state == GB_TIMA_RELOADED) {
        gb->tima_reload_state = GB_TIMA_RUNNING;
    }
    else if (gb->tima_reload_state == GB_TIMA_RELOADING) {
        gb->io_registers[GB_IO_IF] |= 4;
        gb->tima_reload_state = GB_TIMA_RELOADED;
    }
}

static void increase_tima(GB_gameboy_t *gb)
{
    gb->io_registers[GB_IO_TIMA]++;
    if (gb->io_registers[GB_IO_TIMA] == 0) {
        gb->io_registers[GB_IO_TIMA] = gb->io_registers[GB_IO_TMA];
        gb->tima_reload_state = GB_TIMA_RELOADING;
    }
}

static void GB_set_internal_div_counter(GB_gameboy_t *gb, uint16_t value)
{
    /* TIMA increases when a specific high-bit becomes a low-bit. */
    value &= INTERNAL_DIV_CYCLES - 1;
    uint16_t triggers = gb->div_counter & ~value;
    if ((gb->io_registers[GB_IO_TAC] & 4) && (triggers & GB_TAC_TRIGGER_BITS[gb->io_registers[GB_IO_TAC] & 3])) {
        increase_tima(gb);
    }
    
    /* TODO: Can switching to double speed mode trigger an event? */
    uint16_t apu_bit = gb->cgb_double_speed? 0x2000 : 0x1000;
    if (triggers & apu_bit) {
        GB_apu_run(gb);
        GB_apu_div_event(gb);
    }
    else {
        uint16_t secondary_triggers = ~gb->div_counter & value;
        if (secondary_triggers & apu_bit) {
            GB_apu_run(gb);
            GB_apu_div_secondary_event(gb);
        }
    }
    gb->div_counter = value;
}

static void GB_timers_run(GB_gameboy_t *gb, uint8_t cycles)
{
    if (gb->stopped) {
        if (CGB) {
            gb->apu.apu_cycles += 4 << !gb->cgb_double_speed;
        }
        return;
    }
    
    GB_STATE_MACHINE(gb, div, cycles, 1) {
        GB_STATE(gb, div, 1);
        GB_STATE(gb, div, 2);
        GB_STATE(gb, div, 3);
    }
    
    GB_set_internal_div_counter(gb, 0);
main:
    GB_SLEEP(gb, div, 1, 3);
    while (true) {
        advance_tima_state_machine(gb);
        GB_set_internal_div_counter(gb, gb->div_counter + 4);
        gb->apu.apu_cycles += 4 << !gb->cgb_double_speed;
        GB_SLEEP(gb, div, 2, 4);
    }
    
    /* Todo: This is ugly to allow compatibility with 0.11 save states. Fix me when breaking save compatibility */
    {
        div3:
        /* Compensate for lack of prefetch emulation, as well as DIV's internal initial value */
        GB_set_internal_div_counter(gb, 8);
        goto main;
    }
}

void GB_advance_cycles(GB_gameboy_t *gb, uint8_t cycles)
{
    gb->apu.pcm_mask[0] = gb->apu.pcm_mask[1] = 0xFF; // Sort of hacky, but too many cross-component interactions to do it right

    GB_timers_run(gb, cycles);

    if (!gb->cgb_double_speed) {
        cycles <<= 1;
    }
    
    gb->apu_output.sample_cycles += cycles;
    
    GB_apu_run(gb);
}

/* 
   This glitch is based on the expected results of mooneye-gb rapid_toggle test.
   This glitch happens because how TIMA is increased, see GB_set_internal_div_counter.
   According to GiiBiiAdvance, GBC's behavior is different, but this was not tested or implemented.
*/
void GB_emulate_timer_glitch(GB_gameboy_t *gb, uint8_t old_tac, uint8_t new_tac)
{
    /* Glitch only happens when old_tac is enabled. */
    if (!(old_tac & 4)) return;

    unsigned old_clocks = GB_TAC_TRIGGER_BITS[old_tac & 3];
    unsigned new_clocks = GB_TAC_TRIGGER_BITS[new_tac & 3];

    /* The bit used for overflow testing must have been 1 */
    if (gb->div_counter & old_clocks) {
        /* And now either the timer must be disabled, or the new bit used for overflow testing be 0. */
        if (!(new_tac & 4) || gb->div_counter & new_clocks) {
            increase_tima(gb);
        }
    }
}

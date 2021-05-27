#ifndef timing_h
#define timing_h
#include "gb_struct_def.h"

#ifdef GB_INTERNAL
void GB_advance_cycles(GB_gameboy_t *gb, uint8_t cycles);
void GB_emulate_timer_glitch(GB_gameboy_t *gb, uint8_t old_tac, uint8_t new_tac);
bool GB_timing_sync_turbo(GB_gameboy_t *gb); /* Returns true if should skip frame */
void GB_timing_sync(GB_gameboy_t *gb);

enum {
    GB_TIMA_RUNNING = 0,
    GB_TIMA_RELOADING = 1,
    GB_TIMA_RELOADED = 2
};


#define GB_SLEEP(gb, unit, state, cycles) do {\
    (gb)->unit##_cycles -= (cycles) * __state_machine_divisor; \
    if ((gb)->unit##_cycles <= 0) {\
        (gb)->unit##_state = state;\
        return;\
        unit##state:; \
    }\
} while (0)

#define GB_STATE_MACHINE(gb, unit, cycles, divisor) \
static const int __state_machine_divisor = divisor;\
(gb)->unit##_cycles += cycles; \
if ((gb)->unit##_cycles <= 0) {\
    return;\
}\
switch ((gb)->unit##_state)
#endif

#define GB_STATE(gb, unit, state) case state: goto unit##state

#define GB_UNIT(unit) int32_t unit##_cycles, unit##_state

#endif /* timing_h */

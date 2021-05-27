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

#ifndef GB_DISABLE_TIMEKEEPING
static int64_t get_nanoseconds(void)
{
#ifndef _WIN32
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_usec) * 1000 + now.tv_sec * 1000000000L;
#else
    FILETIME time;
    GetSystemTimeAsFileTime(&time);
    return (((int64_t)time.dwHighDateTime << 32) | time.dwLowDateTime) * 100L;
#endif
}

static void nsleep(uint64_t nanoseconds)
{
#ifndef _WIN32
    struct timespec sleep = {0, nanoseconds};
    nanosleep(&sleep, NULL);
#else
    HANDLE timer;
    LARGE_INTEGER time;
    timer = CreateWaitableTimer(NULL, true, NULL);
    time.QuadPart = -(nanoseconds / 100L);
    SetWaitableTimer(timer, &time, 0, NULL, NULL, false);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#endif
}

bool GB_timing_sync_turbo(GB_gameboy_t *gb)
{
    if (!gb->turbo_dont_skip) {
        int64_t nanoseconds = get_nanoseconds();
        if (nanoseconds <= gb->last_sync + (1000000000LL * LCDC_PERIOD / GB_get_clock_rate(gb))) {
            return true;
        }
        gb->last_sync = nanoseconds;
    }
    return false;
}

void GB_timing_sync(GB_gameboy_t *gb)
{
    if (gb->turbo) {
        gb->cycles_since_last_sync = 0;
        return;
    }
    /* Prevent syncing if not enough time has passed.*/
    if (gb->cycles_since_last_sync < LCDC_PERIOD / 3) return;

    uint64_t target_nanoseconds = gb->cycles_since_last_sync * 1000000000LL / 2 / GB_get_clock_rate(gb); /* / 2 because we use 8MHz units */
    int64_t nanoseconds = get_nanoseconds();
    int64_t time_to_sleep = target_nanoseconds + gb->last_sync - nanoseconds;
    if (time_to_sleep > 0 && time_to_sleep < LCDC_PERIOD * 1200000000LL / GB_get_clock_rate(gb)) { // +20% to be more forgiving
        nsleep(time_to_sleep);
        gb->last_sync += target_nanoseconds;
    }
    else {
        if (time_to_sleep < 0 && -time_to_sleep < LCDC_PERIOD * 1200000000LL / GB_get_clock_rate(gb)) {
            // We're running a bit too slow, but the difference is small enough,
            // just skip this sync and let it even out
            return;
        }
        gb->last_sync = nanoseconds;
    }

    gb->cycles_since_last_sync = 0;
    if (gb->update_input_hint_callback) {
        gb->update_input_hint_callback(gb);
    }
}
#else

bool GB_timing_sync_turbo(GB_gameboy_t *gb)
{
    return false;
}

void GB_timing_sync(GB_gameboy_t *gb)
{
}

#endif

#define IR_DECAY 31500
#define IR_THRESHOLD 19900
#define IR_MAX IR_THRESHOLD * 2 + IR_DECAY

static void GB_ir_run(GB_gameboy_t *gb, uint32_t cycles)
{
    if (gb->model == GB_MODEL_AGB) return;
    if (gb->infrared_input || gb->cart_ir || (gb->io_registers[GB_IO_RP] & 1)) {
        gb->ir_sensor += cycles;
        if (gb->ir_sensor > IR_MAX) {
            gb->ir_sensor = IR_MAX;
        }
        
        gb->effective_ir_input = gb->ir_sensor >= IR_THRESHOLD && gb->ir_sensor <= IR_THRESHOLD + IR_DECAY;
    }
    else {
        if (gb->ir_sensor <= cycles) {
            gb->ir_sensor = 0;
        }
        else {
            gb->ir_sensor -= cycles;
        }
        gb->effective_ir_input = false;
    }
    
}

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
        if (GB_is_cgb(gb)) {
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

static void advance_serial(GB_gameboy_t *gb, uint8_t cycles)
{
    if (gb->printer.command_state || gb->printer.bits_received) {
        gb->printer.idle_time += cycles;
    }
    if (gb->serial_length == 0) {
        gb->serial_cycles += cycles;
        return;
    }
    
    while (cycles > gb->serial_length) {
        advance_serial(gb, gb->serial_length);
        cycles -= gb->serial_length;
    }
    
    uint16_t previous_serial_cycles = gb->serial_cycles;
    gb->serial_cycles += cycles;
    if ((gb->serial_cycles & gb->serial_length) != (previous_serial_cycles & gb->serial_length)) {
        gb->serial_count++;
        if (gb->serial_count == 8) {
            gb->serial_length = 0;
            gb->serial_count = 0;
            gb->io_registers[GB_IO_SC] &= ~0x80;
            gb->io_registers[GB_IO_IF] |= 8;
        }
        
        gb->io_registers[GB_IO_SB] <<= 1;
        
        if (gb->serial_transfer_bit_end_callback) {
            gb->io_registers[GB_IO_SB] |= gb->serial_transfer_bit_end_callback(gb);
        }
        else {
            gb->io_registers[GB_IO_SB] |= 1;
        }
        
        if (gb->serial_length) {
            /* Still more bits to send */
            if (gb->serial_transfer_bit_start_callback) {
                gb->serial_transfer_bit_start_callback(gb, gb->io_registers[GB_IO_SB] & 0x80);
            }
        }
        
    }
    return;
    
}

static void GB_rtc_run(GB_gameboy_t *gb, uint8_t cycles)
{
    if (gb->cartridge_type->mbc_type != GB_HUC3 && !gb->cartridge_type->has_rtc) return;
    gb->rtc_cycles += cycles;
    time_t current_time = 0;
    
    switch (gb->rtc_mode) {
        case GB_RTC_MODE_SYNC_TO_HOST:
            // Sync in a 1/32s resolution
            if (gb->rtc_cycles < GB_get_unmultiplied_clock_rate(gb) / 16) return;
            gb->rtc_cycles -= GB_get_unmultiplied_clock_rate(gb) / 16;
            current_time = time(NULL);
            break;
        case GB_RTC_MODE_ACCURATE:
            if (gb->cartridge_type->mbc_type != GB_HUC3 && (gb->rtc_real.high & 0x40)) {
                gb->rtc_cycles -= cycles;
                return;
            }
            if (gb->rtc_cycles < GB_get_unmultiplied_clock_rate(gb) * 2) return;
            gb->rtc_cycles -= GB_get_unmultiplied_clock_rate(gb) * 2;
            current_time = gb->last_rtc_second + 1;
            break;
    }

    if (gb->cartridge_type->mbc_type == GB_HUC3) {
        while (gb->last_rtc_second / 60 < current_time / 60) {
            gb->last_rtc_second += 60;
            gb->huc3_minutes++;
            if (gb->huc3_minutes == 60 * 24) {
                gb->huc3_days++;
                gb->huc3_minutes = 0;
            }
        }
        return;
    }
    bool running = false;
    if (gb->cartridge_type->mbc_type == GB_TPP1) {
        running = gb->tpp1_mr4 & 0x4;
    }
    else {
        running = (gb->rtc_real.high & 0x40) == 0;
    }
    
    if (running) { /* is timer running? */
        while (gb->last_rtc_second + 60 * 60 * 24 < current_time) {
            gb->last_rtc_second += 60 * 60 * 24;
            if (gb->cartridge_type->mbc_type == GB_TPP1) {
                if (++gb->rtc_real.tpp1.weekday == 7) {
                    gb->rtc_real.tpp1.weekday = 0;
                    if (++gb->rtc_real.tpp1.weeks == 0) {
                        gb->tpp1_mr4 |= 8; /* Overflow bit */
                    }
                }
            }
            else if (++gb->rtc_real.days == 0) {
                if (gb->rtc_real.high & 1) { /* Bit 8 of days*/
                    gb->rtc_real.high |= 0x80; /* Overflow bit */
                }
                
                gb->rtc_real.high ^= 1;
            }
        }
        
        while (gb->last_rtc_second < current_time) {
            gb->last_rtc_second++;
            if (++gb->rtc_real.seconds != 60) continue;
            gb->rtc_real.seconds = 0;
            
            if (++gb->rtc_real.minutes != 60) continue;
            gb->rtc_real.minutes = 0;
            
            if (gb->cartridge_type->mbc_type == GB_TPP1) {
                if (++gb->rtc_real.tpp1.hours != 24) continue;
                gb->rtc_real.tpp1.hours = 0;
                if (++gb->rtc_real.tpp1.weekday != 7) continue;
                gb->rtc_real.tpp1.weekday = 0;
                if (++gb->rtc_real.tpp1.weeks == 0) {
                    gb->tpp1_mr4 |= 8; /* Overflow bit */
                }
            }
            else {
                if (++gb->rtc_real.hours != 24) continue;
                gb->rtc_real.hours = 0;
                
                if (++gb->rtc_real.days != 0) continue;
                
                if (gb->rtc_real.high & 1) { /* Bit 8 of days*/
                    gb->rtc_real.high |= 0x80; /* Overflow bit */
                }
                
                gb->rtc_real.high ^= 1;
            }
        }
    }
}


void GB_advance_cycles(GB_gameboy_t *gb, uint8_t cycles)
{
    gb->apu.pcm_mask[0] = gb->apu.pcm_mask[1] = 0xFF; // Sort of hacky, but too many cross-component interactions to do it right
    // Affected by speed boost
    gb->dma_cycles += cycles;

    GB_timers_run(gb, cycles);
    if (!gb->stopped) {
        advance_serial(gb, cycles); // TODO: Verify what happens in STOP mode
    }

    gb->debugger_ticks += cycles;

    if (!gb->cgb_double_speed) {
        cycles <<= 1;
    }
    
    // Not affected by speed boost
    if (gb->io_registers[GB_IO_LCDC] & 0x80) {
        gb->double_speed_alignment += cycles;
    }
    gb->hdma_cycles += cycles;
    gb->apu_output.sample_cycles += cycles;
    gb->cycles_since_last_sync += cycles;
    gb->cycles_since_run += cycles;
    
    gb->rumble_on_cycles += gb->rumble_strength & 3;
    gb->rumble_off_cycles += (gb->rumble_strength & 3) ^ 3;
        
    if (!gb->stopped) { // TODO: Verify what happens in STOP mode
        GB_dma_run(gb);
        GB_hdma_run(gb);
    }
    GB_apu_run(gb);
    GB_display_run(gb, cycles);
    GB_ir_run(gb, cycles);
    GB_rtc_run(gb, cycles);
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

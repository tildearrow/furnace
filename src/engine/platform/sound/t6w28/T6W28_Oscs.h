
// Private oscillators used by T6W28_Apu

// T6W28_Snd_Emu

#ifndef SMS_OSCS_H
#define SMS_OSCS_H

namespace MDFN_IEN_NGP
{

struct T6W28_Osc
{
        blip_buffer_t* outputs[2];
        DivDispatchOscBuffer* oscBuf;
	int output_select;
	
	int delay;
	int last_amp_left;
	int last_amp_right;

	int volume_left;
	int volume_right;

	T6W28_Osc();
	void reset();
};

struct T6W28_Square : T6W28_Osc
{
	int period;
	int phase;
	
	void reset();
	void run( sms_time_t, sms_time_t );
};

struct T6W28_Noise : T6W28_Osc
{
	const int* period;
	int period_extra;
	unsigned shifter;
	unsigned tap;
	
	void reset();
	void run( sms_time_t, sms_time_t );
};

}

#endif


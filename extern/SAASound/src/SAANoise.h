// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// SAANoise.h: interface for the CSAANoise class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SAANOISE_H_INCLUDED
#define SAANOISE_H_INCLUDED

class CSAANoise  
{
private:
	unsigned int m_nCounter;
	uint64_t m_nAdd;
	unsigned int m_nCounter_low;
	unsigned int m_nOversample;
	unsigned int m_nCounterLimit_low;
	bool m_bSync; // see description of "SYNC" bit of register 28
	uint64_t m_nSampleRate; // = 44100 when RateMode=0, for example
	int m_nSourceMode;
	uint64_t m_nAddBase; // nAdd for 31.25 kHz noise at 44.1 kHz samplerate

	// pseudo-random number generator
	unsigned int m_nRand;

	void ChangeLevel(void);
	

public:
	CSAANoise();
	CSAANoise(unsigned int seed);
	~CSAANoise();

	void SetSource(int nSource);
	void Trigger(void);
	void _SetSampleRate(int nSampleRate);
	void _SetOversample(unsigned int oversample);
	void _SetClockRate(int nClockRate);
	void Seed(unsigned int seed);

	void Tick(void);
	int Level(void) const;
	void Sync(bool bSync);

};

inline int CSAANoise::Level(void) const
{
	// returns 0 or 1
	return (m_nRand & 0x00000001);
}


#endif	// SAANOISE_H_INCLUDED

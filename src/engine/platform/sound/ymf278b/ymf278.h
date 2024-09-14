#ifndef YMF278_HH
#define YMF278_HH

#include <vector>
#include <string>
#include <algorithm>

#define UNREACHABLE while (1) assert(false)

using byte = uint8_t;

template<typename T>
const T& YMF_clamp(const T& value, const T& min, const T& max) {
    return std::min(std::max(value, min), max);
}

class MemoryInterface {
public:
	virtual byte operator[](unsigned address) const = 0;
	virtual unsigned getSize() const = 0;
	virtual void write(unsigned address, byte value) = 0;
	virtual void clear(byte value) = 0;
	virtual void setMemoryType(bool memoryType) {};
};

class YMF278Base
{
public:
	YMF278Base(MemoryInterface& memory, int channelCount, int clockDivider, double clockFrequency);
	~YMF278Base();
	int getChannelCount();
	int getClockDivider();
	double getClockFrequency();
	void setClockFrequency(double clockFrequency);
	double getSampleRate();
	virtual void reset();

	void generate(short& fleft, short& fright, short& rleft, short& rright, short* channelBufs = nullptr);

	class Slot final {
	public:
		Slot();
		void reset();
		int compute_rate(int val) const;
		int compute_decay_rate(int val) const;
		unsigned decay_rate(int num, int sample_rate);
		void envelope_next(int sample_rate);
		int16_t compute_vib() const;
		uint16_t compute_am() const;

		template<typename Archive>
		void serialize(Archive& ar, unsigned version);

		uint32_t startaddr;
		uint16_t loopaddr;
		uint16_t endaddr; // Note: stored in 2s complement (0x0000 = 0, 0x0001 = -65536, 0xffff = -1)
		uint32_t step;       // fixed-point frequency step
							// invariant: step == calcStep(OCT, FN)
		uint32_t stepptr;    // fixed-point pointer into the sample
		uint16_t pos;

		int16_t env_vol;

		uint32_t lfo_cnt;

		int16_t DL;
		uint16_t wave;		// wavetable number
		uint16_t FN;		// f-number         TODO store 'FN | 1024'?
		int8_t OCT;		// octave [-8..+7]
		bool PRVB;		// pseudo-reverb
		uint8_t TLdest;		// destination total level
		uint8_t TL;		// total level  (goes towards TLdest)
		uint8_t pan;		// panpot 0..15
		bool ch;            // channel select
		bool keyon;		// slot keyed on
		bool DAMP;
		uint8_t lfo;            // LFO speed 0..7
		uint8_t vib;		// vibrato 0..7
		uint8_t AM;		// AM level 0..7
		uint8_t AR;		// 0..15
		uint8_t D1R;		// 0..15
		uint8_t D2R;		// 0..15
		uint8_t RC;		// rate correction 0..15
		uint8_t RR;		// 0..15

		uint8_t bits;		// width of the samples

		uint8_t state;		// envelope generator state
		bool lfo_active;
	};

protected:
	void keyOnHelper(Slot& slot);

	MemoryInterface& memory;
	std::vector<Slot> slots;

private:
	int16_t getSample(Slot& slot, uint16_t pos) const;
	static uint16_t nextPos(Slot& slot, uint16_t pos, uint16_t increment);
	void advance();
	bool anyActive();

	/** Global envelope generator counter. */
	unsigned eg_cnt;

	unsigned channelCount, clockDivider;
	double clockFrequency;
};

class YMF278 final : public YMF278Base {
public:
	YMF278(MemoryInterface& memory);
	void reset() override;
	void writeReg(byte reg, byte data);
	byte readReg(byte reg);
	byte peekReg(byte reg) const;

	void generateMix(short fmL, short fmR, short& bufFL, short& bufFR, short& bufRL, short& bufRR, short* channelBufs = nullptr) {
		generate(bufFL, bufFR, bufRL, bufRR, channelBufs);
		bufFL = std::min(std::max((pcmMixL * bufFL + fmMixL * fmL) >> 4, -0x8000), 0x7fff);
		bufFR = std::min(std::max((pcmMixR * bufFR + fmMixR * fmR) >> 4, -0x8000), 0x7fff);;
	}

private:
	int fmMixL, fmMixR, pcmMixL, pcmMixR;
	int memAdr;
	byte regs[256];
};

class YMW258 final : public YMF278Base {
public:
	YMW258(MemoryInterface& memory);
	void writeReg(byte channel, byte reg, byte data);
};

class MemoryMoonSound : MemoryInterface {
public:
	MemoryMoonSound(MemoryInterface& rom, MemoryInterface& ram);
	byte operator[](unsigned address) const override;
	unsigned getSize() const override;
	void write(unsigned address, byte value) override;
	void clear(byte value) override;
	void setMemoryType(bool memoryType) override;

private:
	unsigned getRamAddress(unsigned addr) const;

	MemoryInterface& rom;
	MemoryInterface& ram;

	bool memoryType;
};

#endif

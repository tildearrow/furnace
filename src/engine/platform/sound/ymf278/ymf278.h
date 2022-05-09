#ifndef YMF278_HH
#define YMF278_HH

#include <vector>
#include <string>
#include <algorithm>

#define UNREACHABLE while (1) assert(false)

using byte = uint8_t;

namespace std {  // c++17
	template<typename T>
	const T& clamp(const T& value, const T& min, const T& max) {
		return std::min(std::max(value, min), max);
	}
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
	YMF278Base(MemoryInterface& memory, size_t channels);
	~YMF278Base();
	virtual void reset();

	void generate(short& left, short& right, short* channelBufs = nullptr);

	class Slot final {
	public:
		Slot();
		void reset();
		[[nodiscard]] int compute_rate(int val) const;
		[[nodiscard]] int compute_decay_rate(int val) const;
		[[nodiscard]] unsigned decay_rate(int num, int sample_rate);
		void envelope_next(int sample_rate);
		[[nodiscard]] int16_t compute_vib() const;
		[[nodiscard]] uint16_t compute_am() const;

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
	[[nodiscard]] int16_t getSample(Slot& slot, uint16_t pos) const;
	[[nodiscard]] static uint16_t nextPos(Slot& slot, uint16_t pos, uint16_t increment);
	void advance();
	[[nodiscard]] bool anyActive();

	/** Global envelope generator counter. */
	unsigned eg_cnt;
};

class YMF278 final : public YMF278Base {
public:
	YMF278(MemoryInterface& memory);
	void reset() override;
	void writeReg(byte reg, byte data);
	[[nodiscard]] byte readReg(byte reg);
	[[nodiscard]] byte peekReg(byte reg) const;

	void generateMix(short fmL, short fmR, short& bufL, short& bufR, short* channelBufs = nullptr) {
		generate(bufL, bufR, channelBufs);
		bufL = std::min(std::max((pcmMixL * bufL + fmMixL * fmL) >> 4, -0x8000), 0x7fff);
		bufR = std::min(std::max((pcmMixR * bufR + fmMixR * fmR) >> 4, -0x8000), 0x7fff);;
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
	[[nodiscard]] unsigned getRamAddress(unsigned addr) const;

	MemoryInterface& rom;
	MemoryInterface& ram;

	bool memoryType;
};

#endif

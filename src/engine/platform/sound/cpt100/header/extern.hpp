#ifndef CPT100_EXTERN_HPP_
#define CPT100_EXTERN_HPP_

namespace cpt100 {
    class Byte {
    public:
        Byte(int val);
        int toInt();
    private:
        int value;
    };
    extern std::vector<Byte> ram;
    extern std::vector<Byte> vram;
    extern void ram_boot(std::vector<Byte>& ram, std::vector<Byte>& vram);
    extern Byte vram_peek(std::vector<Byte>& vram, int addr);
    extern void vram_poke(std::vector<Byte>& vram, int addr, Byte val);
    extern std::vector<Byte> vram_peek2array(std::vector<Byte>& vram, int addr, int block);
    extern void vram_pokefill(std::vector<Byte>& vram, int addr, int block, Byte val);
    extern void vram_poke2array(std::vector<Byte>& vram, int addr, std::vector<Byte>& vals);
    extern Byte ram_peek(std::vector<Byte>& ram, int addr);
    extern void ram_poke(std::vector<Byte>& ram, int addr, Byte val);
    extern std::vector<Byte> ram_peek2array(std::vector<Byte>& ram, int addr, int block);
    extern void ram_pokefill(std::vector<Byte>& ram, int addr, int block, Byte val);
    extern void ram_poke2array(std::vector<Byte>& ram, int addr, std::vector<Byte>& vals);
    struct ADSRConfig{
        double attackTime;
		double decayTime;
		double sustainLevel;
		double releaseTime;
    };
	class EnvGenerator
	{
	public:
		enum class State
		{
			Attack, Decay, Sustain, Release
		};
		void noteOff();
		void reset(State state);
		void update(const ADSRConfig& adsr, double dt);
		bool isReleased(const ADSRConfig& adsr);
		double currentLevel();
		State state();
		double lerp(double start, double end, double t);
		State m_state;
		double m_elapsed; // ステート変更からの経過秒数
		double m_currentLevel; // 現在のレベル [0, 1]
		double m_lastLevel;
	};
    extern double sind(double theta);
    extern double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4);
    extern void applyEnveloveToRegisters(std::vector<Byte> &reg, std::vector<Byte> &regenvl, int opNum, int ch, double dt);
    extern std::vector<int16_t> AudioCallBack(int len);
    extern void initSound();
    extern void resetGate(int ch);
}

#endif // CPT100_EXTERN_HPP_
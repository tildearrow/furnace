// BR811 - nucleo de emulacao do chip de som do console ficticio Ompreza (1984)
//
// Nucleo independente, sem dependencia do Furnace Tracker. A ideia e plugar
// esta classe depois na camada "platform" do Furnace (ver passo 4 do roteiro).
// Todas as constantes vem do documento "BR811 - Especificacao do chip de som".
#pragma once

#include <cstdint>
#include <functional>

namespace br811 {

// ---------------------------------------------------------------------------
// Especificacao
// ---------------------------------------------------------------------------
constexpr double  CLOCK_HZ      = 6420000.0;  // clock do chip = clock da CPU RS04
constexpr int     NUM_CHANNELS  = 5;
constexpr int     DIVISOR       = 57;         // divisor de afinacao (era 114: dobra a resolucao da afinacao)
constexpr int     MAX_PERIOD    = 4095;       // periodo de 12 bits
constexpr int     VOLUME_MAX    = 15;         // volume de 0 a 15

constexpr int     WAVE_A_STEPS  = 8;
constexpr int     WAVE_B_STEPS  = 16;

constexpr int     PCM_CHANNEL         = 4;    // so o canal 5 (indice 4) faz PCM
constexpr int     PCM_CYCLES_PER_BYTE = 16;   // ciclos de CPU roubados por byte lido
constexpr int     PCM_BLOCK_BYTES     = 16;   // tamanho do sample em blocos de 16 bytes
constexpr int     PCM_REF_PERIOD      = 430;  // periodo de C-4: PCM toca a 8.000 Hz

// Unidade interna de tempo: 1/1024 de ciclo de clock. Permite representar
// com inteiros as fracoes que o divisor 57 gera (7,125 e 3,5625 clocks por passo).
constexpr int64_t UNITS_PER_CLOCK = 1024;

// ---------------------------------------------------------------------------
// Registradores (rascunho do mapa de registradores, secao 8 da especificacao)
//
// Cada canal tem 3 registradores:
//   reg 0: bits 7-6 modo | bits 5-4 livres | bits 3-0 volume
//   reg 1: periodo, 8 bits baixos
//   reg 2: bit 7 livre | bits 6-4 taxa de ruido | bits 3-0 periodo, 4 bits altos
//
// Registradores extras do PCM (canal 5):
//   0: endereco inicial, byte baixo     2: tamanho ((valor + 1) * 16 bytes)
//   1: endereco inicial, byte alto      3: controle (bit 0 = tocar, bit 1 = loop)
// ---------------------------------------------------------------------------
enum Mode : uint8_t {
    MODE_WAVE_A = 0,
    MODE_WAVE_B = 1,
    MODE_NOISE  = 2,
    MODE_PCM    = 3,   // so no canal 5; reservado (silencio) nos demais
};

// Taxa de ruido: campo de 3 bits. Os codigos 6 e 7 sao reservados (agem como 0).
enum NoiseRate : uint8_t {
    NOISE_DIV512  = 0,  // clock / 512  (cerca de 12.539 Hz)
    NOISE_DIV1024 = 1,  // clock / 1024 (cerca de  6.270 Hz)
    NOISE_DIV2048 = 2,  // clock / 2048 (cerca de  3.135 Hz)
    NOISE_LINKED  = 3,  // acompanha o periodo do proprio canal
    NOISE_DIV256  = 4,  // clock / 256  (cerca de 25.078 Hz) - chimbal aberto, pratos
    NOISE_DIV128  = 5,  // clock / 128  (cerca de 50.156 Hz) - chimbal fechado, pratos agudos
};

enum PcmReg : int {
    PCM_ADDR_LO = 0,
    PCM_ADDR_HI = 1,
    PCM_LENGTH  = 2,
    PCM_CTRL    = 3,
};
constexpr uint8_t PCM_CTRL_PLAY = 0x01;
constexpr uint8_t PCM_CTRL_LOOP = 0x02;

// Periodo (12 bits) mais proximo para uma frequencia em Hz.
int periodForFrequency(double hz);
// Periodo para uma nota MIDI (60 = C-4, 69 = A-4 = 440 Hz).
int periodForMidi(int midiNote);

// ---------------------------------------------------------------------------
// O chip
// ---------------------------------------------------------------------------
class Chip {
public:
    // Le um byte do cartucho. O Ompreza tem barramento de 16 bits.
    using RomReader = std::function<uint8_t(uint16_t)>;

    explicit Chip(double sampleRate = 48000.0);

    void reset();
    // Muda a taxa de saida (usado pelo Furnace: clock / 57).
    void setSampleRate(double sampleRate);
    void setRomReader(RomReader reader) { rom_ = std::move(reader); }

    // Escrita nos registradores de um canal (channel 0..4, reg 0..2).
    void write(int channel, int reg, uint8_t value);
    // Escrita nos registradores do PCM (reg = PcmReg).
    void writePcm(int reg, uint8_t value);

    // Gera numSamples amostras mono, em float de 0.0 a 1.0 (sinal unipolar,
    // como sai do chip; remova o nivel DC ao gravar em arquivo).
    // Se perChannel for informado (array de NUM_CHANNELS ponteiros), cada canal
    // tambem recebe a propria saida, de 0.0 a 1.0 (usado nos osciloscopios do Furnace).
    void render(float* out, int numSamples, float* const* perChannel = nullptr);

    // Ciclos de CPU roubados pelo DMA do PCM desde o reset.
    uint64_t stolenCycles() const { return stolenCycles_; }
    // Ciclos de clock emulados desde o reset.
    uint64_t elapsedCycles() const { return static_cast<uint64_t>(totalUnits_ / UNITS_PER_CLOCK); }

    // Depuracao e testes.
    uint16_t noiseState(int channel) const { return ch_[channel].lfsr; }
    uint64_t noiseShifts(int channel) const { return noiseShifts_[channel]; }

private:
    struct Channel {
        uint8_t  mode      = MODE_WAVE_A;
        uint8_t  noiseRate = NOISE_DIV512;
        uint8_t  volume    = 0;
        uint16_t period    = 0;
        int64_t  timer     = 1;       // unidades de tempo ate o proximo passo
        int      step      = 0;       // posicao dentro da onda
        uint16_t lfsr      = 0x8000;  // registrador de ruido de 16 bits
    };

    struct Pcm {
        uint16_t addr    = 0;
        uint8_t  length  = 0;
        bool     playing = false;
        bool     loop    = false;
        uint32_t offset  = 0;
        uint8_t  sample  = 0;         // ultimo byte lido (7 bits uteis)
    };

    int64_t stepPeriod(const Channel& c, int index) const;
    float   level(const Channel& c, int index) const;
    void    advance(Channel& c, int index);
    double  runChannel(Channel& c, int index, int64_t dt);

    void pcmFetch();
    void pcmNext();

    double   sampleRate_;
    double   unitsPerSample_;
    double   unitsAccum_ = 0.0;
    int64_t  totalUnits_ = 0;
    uint64_t stolenCycles_ = 0;
    uint64_t noiseShifts_[NUM_CHANNELS] = {};

    Channel  ch_[NUM_CHANNELS];
    Pcm      pcm_;
    RomReader rom_;
};

}  // namespace br811

#include <vector>
#include "header/spec.hpp"
#include "header/types.hpp"

std::vector<Byte> ram;
std::vector<Byte> vram;

// RAMおよびVRAMを管理する関数
void ram_boot(std::vector<Byte>& ram, std::vector<Byte>& vram) {
    ram.resize(CPT100_RAM_SIZE, Byte(0));
    vram.resize(CPT100_VRAM_SIZE, Byte(0));
}

Byte vram_peek(std::vector<Byte>& vram, int addr) {
    if (addr < 0) {
        return Byte(0);
    }
    if (addr < CPT100_VRAM_SIZE) {
        return vram.at(addr);
    }
    return Byte(0);
}

void vram_poke(std::vector<Byte>& vram, int addr, Byte val) {
    if (addr < 0) {
        return;
    }   
    if (addr < CPT100_VRAM_SIZE) {
        vram.at(addr) = val;
    }
}

std::vector<Byte> vram_peek2array(std::vector<Byte>& vram, int addr, int block) {
    std::vector<Byte> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(vram_peek(vram, i));
    }
    return out;
}

void vram_pokefill(std::vector<Byte>& vram, int addr, int block, Byte val) {
    std::fill(vram.begin() + addr, vram.begin() + addr + block, val);
}

void vram_poke2array(std::vector<Byte>& vram, int addr, std::vector<Byte>& vals) {
    std::copy(vals.begin(), vals.end(), vram.begin() + addr);
}

Byte ram_peek(std::vector<Byte>& ram, int addr) {
    if (addr < 0) {
        return Byte(0);
    }
    if (addr < CPT100_RAM_SIZE) {
        return ram.at(addr);
    }
    return Byte(0);
}

void ram_poke(std::vector<Byte>& ram, int addr, Byte val) {
    if (addr < 0) {
        return;
    }   
    if (addr < CPT100_RAM_SIZE) {
        ram.at(addr) = val;
    }
}

std::vector<Byte> ram_peek2array(std::vector<Byte>& ram, int addr, int block) {
    std::vector<Byte> out;
    for (int i = addr; i < addr + block; i++)
    {
        out.push_back(ram_peek(ram, i));
    }
    return out;
}

void ram_pokefill(std::vector<Byte>& ram, int addr, int block, Byte val) {
    std::fill(ram.begin() + addr, ram.begin() + addr + block, val);
}

void ram_poke2array(std::vector<Byte>& ram, int addr, std::vector<Byte>& vals) {
    std::copy(vals.begin(), vals.end(), ram.begin() + addr);
}


# Konami K005289

## Summary

- 2 12 bit timer
- Address generator

## Source code

- k005289.hpp: Base header
  - k005289.cpp: Source emulation core

## Description

This chip is used at infamous Konami Bubble System, for part of Wavetable sound generator. But seriously, It is just to 2 internal 12 bit timer and address generators, rather than sound generator.

Everything except for internal counter and address are done by external logic, the chip is only has external address, frequency registers and its update pins.

## Frequency calculation

```
Input clock / (4096 - Pitch input)
```

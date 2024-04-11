# MODIFIED

this is a modified version of ESFMu which adds a "fast" mode that uses OPL3 feedback calculation rather than patent workaround calculation (which is slow but accurate).

# ESFMu

An emulator for the ESS "ESFM" enhanced OPL3 clone, based on Nuke.YKT's **Nuked OPL3** and reverse-engineering efforts from the community.

## Acknowledgements

I'd like to thank:

- **Nuke.YKT**
  - Developer of **Nuked OPL3**, which was the basis for **ESFMu**'s code and also a great learning resource on Yamaha FM synthesis for myself.
  - Nuke.YKT also gives shoutouts on behalf of **Nuked OPL3** to:
    >- MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
    >    - Feedback and Rhythm part calculation information.
    >- forums.submarine.org.uk(carbon14, opl3):
    >    - Tremolo and phase generator calculation information.
    >- OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
    >    - OPL2 ROMs.
    >- siliconpr0n.org(John McMaster, digshadow):
    >    - YMF262 and VRC VII decaps and die shots.
- **rainwarrior**
  - For performing the initial research on ESFM drivers and documenting ESS's patent on native mode operator organization.
- **jwt27**
  - For kickstarting the ESFM research project and compiling rainwarrior's findings and more in an accessible document ("ESFM Demystified").
- **pachuco/CatButts**
  - For documenting ESS's patent on ESFM's feedback implementation, which was vital in getting **ESFMu**'s sound output to be accurate.
- And everybody who helped out with real hardware testing

## Usage

To use **ESFMu**:

- include the **esfm.h** header file into your source code
- include the **esfm.c** and **esfm_registers.c** files into your build and link process
- declare or allocate a variable of type `esfm_chip` somewhere in your code - this will hold the chip's state
- use the function interface defined in **esfm.h** to interact with the `esfm_chip` structure

## Function interface

If you're familiar with **Nuked OPL3**, you'll find many similarities in the function interface provided by **ESFMu**. There are a few things to point out, however:

### Buffered writes

Just like **Nuked OPL3**, **ESFMu** offers buffered register writes. However, it offers them in two flavors: "legacy" and fast.

The fast buffered register writes (`ESFM_write_reg_buffered_fast`) are recommended, since they offer minimal latency which is close to the behavior you'd get with the actual ESS drivers on Windows.

The "legacy" buffered register writes are only recommended for specific cases, such as programs seeking for a shortcut to emulate the write delays from some sound drivers.

### Port-level access

Unlike **Nuked OPL3**, **ESFMu** actually allows port-level access to the ESFM interface. This is relevant because the ESFM port interface is actually modal, meaning that its behavior changes depending on whether the chip is set to emulation (OPL3 compatibility) mode or native (ESFM) mode.

Using port-level access allows for applications to not need to keep track of whether the chip is in native mode or not, nor to perform the port handling logic on their side.

Applications that use the register-level access, on the other hand, need to take care to either stick to only one of the operating modes (either native or emulation), or handle the port mapping logic on their own side.

### Register readback

ESFM allows for register contents to be read back through its ports, and **ESFMu** implements this functionality, both via dedicated register read functions and via the port read interface.

Note that in ESFM, register contents can only be read back when the chip is set to native (ESFM) mode, not when the chip is in emulation mode (i.e. OPL3 compatibility mode).

## Licensing

**ESFMu** is highly based on **Nuked OPL3**, which is licensed under the GNU Lesser General Public License version 2.1 or later. Therefore, **ESFMu** is licensed under the same license.

If you'd like to obtain a grant to use **ESFMu** under different terms, you should get in contact with [Nuke.YKT](https://github.com/nukeykt) (author of **Nuked OPL3**) as well as with [Kagamiin~](https://github.com/Kagamiin) (yours truly).

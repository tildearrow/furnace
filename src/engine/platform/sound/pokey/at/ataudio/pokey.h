//	Altirra - Atari 800/800XL emulator
//	Copyright (C) 2008 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef AT_POKEY_H
#define AT_POKEY_H

#include <stdint.h>
#include <deque>
#include <vector>

#ifdef _MSC_VER
	#pragma once
#endif

class IATAudioOutput;
class ATPokeyEmulator;
class ATSaveStateReader;
class ATAudioFilter;
struct ATPokeyTables;
class ATPokeyRenderer;
class IATObjectState;

class IATPokeyEmulatorConnections {
public:
	virtual void PokeyAssertIRQ(bool cpuBased) = 0;
	virtual void PokeyNegateIRQ(bool cpuBased) = 0;
	virtual void PokeyBreak() = 0;
	virtual bool PokeyIsInInterrupt() const = 0;
	virtual bool PokeyIsKeyPushOK(uint8_t scanCode, bool cooldownExpired) const = 0;
};

class IATPokeySIODevice {
public:
	virtual void PokeyAttachDevice(ATPokeyEmulator *pokey) = 0;

	// Returns true if burst I/O is allowed.
	virtual bool PokeyWriteSIO(uint8_t c, bool command, uint32_t cyclesPerBit, uint64_t startTime, bool framingError) = 0;

	virtual void PokeyBeginCommand() = 0;
	virtual void PokeyEndCommand() = 0;
	virtual void PokeySerInReady() = 0;
};

class IATPokeyCassetteDevice {
public:
	virtual void PokeyChangeSerialRate(uint32_t divisor) = 0;
	virtual void PokeyResetSerialInput() = 0;
	virtual void PokeyBeginCassetteData(uint8_t skctl) = 0;
	virtual bool PokeyWriteCassetteData(uint8_t c, uint32_t cyclesPerBit) = 0;
};

class IATPokeyTraceOutput {
public:
	virtual void AddIRQ(uint64_t start, uint64_t end) = 0;
};

struct ATPokeyRegisterState {
	uint8_t mReg[0x20];
};

struct ATPokeyAudioState {
	int		mChannelOutputs[4];
};

struct ATPokeyAudioLog {
	// Sampling buffer -- receives per-channel output state every N ticks, up to the given max
	// number of samples per frame. Automatically cleared per frame.
	ATPokeyAudioState	*mpStates;
	uint32_t	mMaxSamples;
	uint32_t	mCyclesPerSample;

	// Mixed sampling buffer -- receives combined output state every sample, up to the given max
	// samples per frame. This buffer is NOT automatically cleared and must be manually retriggered.
	float	*mpMixedSamples;
	uint32_t	mMaxMixedSamples;

	// === filled in by audio engine ===
	uint32_t	mFullScaleValue;
	uint32_t	mTicksPerSample;
	uint32_t	mLastFrameSampleCount;
	uint32_t	mNumMixedSamples;

	// === for continuous use by audio engine ===
	uint32_t	mStartingAudioTick;
	uint32_t	mLastAudioTick;
	uint32_t	mAccumulatedAudioTicks;
	uint32_t	mSampleIndex;
	uint32_t	mLastOutputMask;
};

class ATPokeyEmulator {
public:
	ATPokeyEmulator(bool isSlave);
	~ATPokeyEmulator();

	void	Init(IATPokeyEmulatorConnections *mem, IATAudioOutput *output, ATPokeyTables *tables);
	void	ColdReset();

	void	SetSlave(ATPokeyEmulator *slave);
	void	SetCassette(IATPokeyCassetteDevice *dev);
	void	SetAudioLog(ATPokeyAudioLog *log);
	void	SetConsoleOutput(ATConsoleOutput *output);

	void	Set5200Mode(bool enable);

	bool	IsTraceSIOEnabled() const { return mbTraceSIO; }
	void	SetTraceSIOEnabled(bool enable) { mbTraceSIO = enable; }

	void	AddSIODevice(IATPokeySIODevice *device);
	void	RemoveSIODevice(IATPokeySIODevice *device);

	void	ReceiveSIOByte(uint8_t byte, uint32_t cyclesPerBit, bool simulateInputPort, bool allowBurst, bool synchronous, bool forceFramingError);
	void	SetSERIN(uint8_t v) { mSERIN = v; }

	void	SetAudioLine2(int v);		// used for audio from motor control line
	void	SetDataLine(bool newState, uint64_t flipTime = ~uint64_t(0));
	void	SetCommandLine(bool newState);
	void	SetSpeaker(bool newState);
	void	SetStereoSoftEnable(bool enable);

	void	SetExternalSerialClock(uint32_t basetime, uint32_t period);
	uint32_t	GetSerialCyclesPerBitRecv() const;
	uint32_t	GetSerialInputResetCounter() const { return mSerialInputResetCounter; }

	bool	IsChannelEnabled(uint32_t channel) const;
	void	SetChannelEnabled(uint32_t channel, bool enabled);

	bool	IsSecondaryChannelEnabled(uint32_t channel) const;
	void	SetSecondaryChannelEnabled(uint32_t channel, bool enabled);

	bool	IsNonlinearMixingEnabled() const { return mbNonlinearMixingEnabled; }
	void	SetNonlinearMixingEnabled(bool enable);

	bool	IsSerialNoiseEnabled() const { return mbSerialNoiseEnabled; }
	void	SetSerialNoiseEnabled(bool enable) { mbSerialNoiseEnabled = enable; }

	bool	GetShiftKeyState() const { return mbShiftKeyState; }
	void	SetShiftKeyState(bool down, bool immediate);
	bool	GetControlKeyState() const { return mbControlKeyState; }
	void	SetControlKeyState(bool down);
	void	ClearKeyQueue();
	void	PushKey(uint8_t c, bool repeat, bool allowQueue = false, bool flushQueue = true, bool useCooldown = true);
	uint64_t	GetRawKeyMask() const;
	void	PushRawKey(uint8_t c, bool immediate);
	void	ReleaseRawKey(uint8_t c, bool immediate);
	void	ReleaseAllRawKeys(bool immediate);
	void	SetBreakKeyState(bool down, bool immediate);
	void	PushBreak();

	void	SetKeyMatrix(const bool matrix[64]);

	void	SetPotPos(unsigned idx, int pos);
	void	SetPotPosHires(unsigned idx, int pos, bool grounded);

	// Get/set immediate pot mode. Immediate pot mode allows the POT0-7 registers
	// to update within a frame of the last pot scan triggered via POTGO. This
	// fibs accuracy slightly for reduction in latency.
	bool	IsImmediatePotUpdateEnabled() const { return mbAllowImmediatePotUpdate; }
	void	SetImmediatePotUpdateEnabled(bool enabled) { mbAllowImmediatePotUpdate = enabled; }

	void	AdvanceScanLine();
	void	AdvanceFrame(bool pushAudio, uint64_t timestamp);

	uint8_t	DebugReadByte(uint8_t reg) const;
	uint8_t	ReadByte(uint8_t reg);
	void	WriteByte(uint8_t reg, uint8_t value);

	void	DumpStatus(ATConsoleOutput& out);

	void	SaveState(IATObjectState **pp);
	void	LoadState(const IATObjectState& state);
	void	PostLoadState();

	void	GetRegisterState(ATPokeyRegisterState& state) const;

	void	FlushAudio(bool pushAudio, uint64_t timestamp);

	void	SetTraceOutput(IATPokeyTraceOutput *output);

	uint32_t	GetCyclesToTimerFire(uint32_t ch) const;

protected:
  // override
	void	OnScheduledEvent(uint32_t id);

	void	PostFrameUpdate(uint32_t t);

	template<uint8_t channel>
	void	FireTimer();

	uint32_t	UpdateLast15KHzTime();
	uint32_t	UpdateLast15KHzTime(uint32_t t);
	uint32_t	UpdateLast64KHzTime();
	uint32_t	UpdateLast64KHzTime(uint32_t t);

	void	UpdatePolyTime();

	void	OnSerialInputTick();
	void	OnSerialOutputTick();
	bool	IsSerialOutputClockRunning() const;
	void	FlushSerialOutput();

	void	RecomputeAllowedDeferredTimers();

	template<int channel>
	void	RecomputeTimerPeriod();

	template<int channel>
	void	UpdateTimerCounter();

	void	SetupTimers(uint8_t channels);
	void	FlushDeferredTimerEvents(int channel);
	void	SetupDeferredTimerEvents(int channel, uint32_t t0, uint32_t period);
	void	SetupDeferredTimerEventsLinked(int channel, uint32_t t0, uint32_t period, uint32_t hit0, uint32_t hiperiod, uint32_t hilooffset);

	void	DumpStatus(ATConsoleOutput& out, bool isSlave);

	void	UpdateMixTable();

	void	UpdateKeyMatrix(int index, uint16_t mask, uint16_t state);
	void	UpdateEffectiveKeyMatrix();
	bool	CanPushKey(uint8_t scanCode) const;
	void	TryPushNextKey();

	void	SetKeyboardModes(bool cooked, bool scanEnabled);
	void	UpdateKeyboardScanEvent();
	void	QueueKeyboardIRQ();
	void	AssertKeyboardIRQ();
	void	AssertBreakIRQ();
	void	AssertIrq(bool cpuBased);
	void	NegateIrq(bool cpuBased);

	void	ProcessReceivedSerialByte();
	void	SyncRenderers(ATPokeyRenderer *r);

	void	StartPotScan();
	void	UpdatePots(uint32_t timeSkew);

	void	UpdateAddressDecoding();	

private:
	ATPokeyRenderer *mpRenderer;

	int		mTimerCounters[4];

	bool	mbCommandLineState;
	bool	mbPal;
	bool	mb5200Mode;
	bool	mbTraceSIO;
	bool	mbNonlinearMixingEnabled;
	bool	mbSerialNoiseEnabled = true;

	uint8_t	mKBCODE;
	uint32_t	mKeyCodeTimer;
	uint32_t	mKeyCooldownTimer;
	bool	mbKeyboardIRQPending;
	bool	mbUseKeyCooldownTimer;
	bool	mbCookedKeyMode;
	bool	mbKeyboardScanEnabled;
	bool	mbShiftKeyState;
	bool	mbShiftKeyLatchedState;
	bool	mbControlKeyState;
	bool	mbControlKeyLatchedState;
	bool	mbBreakKeyState;
	bool	mbBreakKeyLatchedState;

	uint8_t	mAddressMask;
	uint8_t	mIRQEN;
	uint8_t	mIRQST;
	uint8_t	mAUDF[4];		// $D200/2/4/6: audio frequency, channel 1/2/3/4
	uint8_t	mAUDC[4];		// $D201/3/5/7: audio control, channel 1/2/3/4
	uint8_t	mAUDCTL;		// $D208
							// bit 7: use 9-bit poly instead of 17-bit poly
							// bit 6: clock channel 1 with 1.79MHz instead of 64KHz
							// bit 5: clock channel 3 with 1.79MHz instead of 64KHz
							// bit 4: clock channel 2 with channel 1 instead of 64KHz
							// bit 3: clock channel 4 with channel 3 instead of 64KHz
							// bit 2: apply high pass filter to channel 1 using channel 3
							// bit 1: apply high pass filter to channel 2 using channel 4
							// bit 0: change 64KHz frequency to 15KHz
	uint8_t	mSERIN;			// $D20D: SERIN
	uint8_t	mSEROUT;		// $D20D: SEROUT
	uint8_t	mSKSTAT;		// $D20F: SKSTAT
							// bit 3: shift key depressed
							// bit 2: key depressed
	uint8_t	mSKCTL;			// $D20F: SKCTL
							// bit 3: shift key depressed
							// bit 2: key depressed

	ATPokeyRegisterState mState;

	// countdown timer values
	int		mAUDFP1[4];		// AUDF values, plus 1 (we use these everywhere)
	int		mCounter[4];
	int		mCounterBorrow[4];
	uint32_t	mTimerPeriod[4];
	uint32_t	mTimerFullPeriod[2];		// time for timer to count off 256 in linked mode (#1 and #3 only)

	mutable uint32_t	mLastPolyTime;
	mutable uint32_t	mPoly17Counter;
	mutable uint32_t	mPoly9Counter;
	uint64_t	mPolyShutOffTime;

	uint64_t	mSerialOutputStartTime;
	uint8_t	mSerialInputShiftRegister;
	uint8_t	mSerialOutputShiftRegister;
	uint8_t	mSerialInputCounter;
	uint8_t	mSerialOutputCounter;
	uint8_t	mSerialInputPendingStatus;
	bool	mbSerOutValid;
	bool	mbSerShiftValid;
	bool	mbSerialOutputState;
	bool	mbSpeakerActive;
	bool	mbSerialRateChanged;
	bool	mbSerialWaitingForStartBit;
	bool	mbSerInBurstPendingIRQ1;
	bool	mbSerInBurstPendingIRQ2;
	bool	mbSerInBurstPendingData;
	bool	mbSerInDeferredLoad;
	uint32_t	mSerOutBurstDeadline;
	uint32_t	mSerialInputResetCounter = 0;

	uint32_t	mSerialSimulateInputBaseTime;
	uint32_t	mSerialSimulateInputCyclesPerBit;
	uint32_t	mSerialSimulateInputData;
	bool	mbSerialSimulateInputPort;

	uint32_t	mSerialExtBaseTime;
	uint32_t	mSerialExtPeriod;

	uint64_t	mSerialDataInFlipTime = ~(uint64_t)0;

	ATPokeyTables *mpTables = nullptr;

	// AUDCTL breakout
	bool	mbFastTimer1;
	bool	mbFastTimer3;
	bool	mbLinkedTimers12;
	bool	mbLinkedTimers34;
	bool	mbUse15KHzClock;

	bool	mbAllowDeferredTimer[4];

	uint32_t	mLast15KHzTime;
	uint32_t	mLast64KHzTime;

	bool	mbDeferredTimerEvents[4];
	uint32_t	mDeferredTimerStarts[4];
	uint32_t	mDeferredTimerPeriods[4];

	uint16_t	mKeyMatrix[8] = {};
	uint16_t	mEffectiveKeyMatrix[8] = {};

	IATPokeyEmulatorConnections *mpConn;
	IATPokeyTraceOutput *mpTraceOutput = nullptr;
	ATPokeyEmulator	*mpSlave;
	const bool	mbIsSlave;
	bool	mbIrqAsserted;

	IATAudioOutput *mpAudioOut = nullptr;

	typedef std::vector<IATPokeySIODevice *> Devices;
	Devices	mDevices;

	IATPokeyCassetteDevice *mpCassette = nullptr;

	std::deque<uint8_t> mKeyQueue;

	uint8_t	mKeyScanState = 0;
	uint8_t	mKeyScanCode = 0;
	uint8_t	mKeyScanLatch = 0;

	uint8_t	mPotPositions[8] = {};
	uint8_t	mPotHiPositions[8] = {};
	uint8_t	mPotLatches[8] = {};
	uint8_t	mALLPOT = 0;
	uint8_t	mPotMasterCounter = 0;
	uint64_t	mPotLastScanTime = 0;		// cycle time of last write to POTGO
	uint32_t	mPotLastTimeFast = 0;
	uint32_t	mPotLastTimeSlow = 0;

	bool	mbAllowImmediatePotUpdate = false;
	bool	mbStereoSoftEnable = true;

	bool	mTraceDirectionSend = false;
	uint32_t	mTraceByteIndex = 0;

	bool	mbTraceIrqPending = false;
	uint64_t	mTraceIrqStart = 0;
};

#endif

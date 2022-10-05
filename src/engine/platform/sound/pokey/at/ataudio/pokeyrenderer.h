//	Altirra - Atari 800/800XL/5200 emulator
//	Copyright (C) 2008-2019 Avery Lee
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

#ifndef f_AT_POKEYRENDERER_H
#define f_AT_POKEYRENDERER_H

#include <stdint.h>
#include <deque>
#include <vector>

class ATScheduler;
struct ATPokeyTables;
struct ATPokeyAudioState;
class IATSyncAudioEdgePlayer;
struct ATPokeyAudioLog;
struct ATSaveStatePokeyRenderer;

class ATPokeyRenderer {
	ATPokeyRenderer(const ATPokeyRenderer&) = delete;
	ATPokeyRenderer& operator=(const ATPokeyRenderer&) = delete;
public:
	ATPokeyRenderer();
	~ATPokeyRenderer();

	void Init(ATPokeyTables *tables);
	void ColdReset();

	void SyncTo(const ATPokeyRenderer& src);

	bool GetChannelOutput(int index) const { return (mChannelOutputMask & (1 << index)) != 0; }
	const float *GetOutputBuffer() const { return mRawOutputBuffer; }

	bool IsChannelEnabled(int channel) const { return mbChannelEnabled[channel]; }
	void SetChannelEnabled(int channel, bool enable);

	void SetAudioLog(ATPokeyAudioLog *log);
	void RestartAudioLog(bool initial = false);

	void SetFiltersEnabled(bool enable);
	void SetInitMode(bool init);
	bool SetSpeaker(bool state);
	void SetAudioLine2(int v);

	void ResetTimers();
	void SetAUDCx(int index, uint8_t value);
	void SetAUDCTL(uint8_t value);

	void AddChannelEvent(int channel);
	void SetChannelDeferredEvents(int channel, uint32_t start, uint32_t period);
	void SetChannelDeferredEventsLinked(int channel, uint32_t loStart, uint32_t loPeriod, uint32_t hiStart, uint32_t hiPeriod, uint32_t loOffset);
	void ClearChannelDeferredEvents(int channel, uint32_t t);

	void AddSerialNoisePulse(uint32_t t);

	void StartBlock();

	struct EndBlockInfo {
		uint32_t mTimestamp;
		uint32_t mSamples;
	};

	EndBlockInfo EndBlock(IATSyncAudioEdgePlayer *edgePlayer);

	void LoadState(const ATSaveStatePokeyRenderer& state);
	ATSaveStatePokeyRenderer SaveState() const;

protected:
	enum class ChangeType : uint8_t {
		Audc0,
		Audc1,
		Audc2,
		Audc3,
		Audctl,
		Init,
		ResetOutputs,
		Flush
	};

	void QueueChangeEvent(ChangeType type, uint8_t value);
	void ProcessChangeEvents(uint32_t t);

	void FlushDeferredEvents(int channel, uint32_t t);
	void Flush(const uint32_t t);
	void Flush2(const uint32_t t);
	static void MergeOutputEvents(const uint32_t* src1, const uint32_t* src2, uint32_t* dst);

	typedef std::pair<uint32_t *, const uint32_t *> (ATPokeyRenderer::*FireTimerRoutine)(uint32_t* dst, const uint32_t* src, uint32_t timeBase, uint32_t timeLimit);
	FireTimerRoutine GetFireTimerRoutine(int ch) const;
	template<int activeChannel>
	FireTimerRoutine GetFireTimerRoutine() const;

	template<int activeChannel, uint8_t audcn, bool outputAffectsSignal, bool T_UsePoly9>
	std::pair<uint32_t *, const uint32_t *> FireTimer(uint32_t* dst, const uint32_t* src, uint32_t timeBase, uint32_t timeLimit);

	void ProcessOutputEdges(uint32_t timeBase, const uint32_t *edges, uint32_t n);
	void UpdateVolume(int channel);
	void UpdateOutput(uint32_t t);
	void UpdateOutput2(uint32_t t2);
	void UpdateOutput2(uint32_t t2, uint32_t vpok);
	void GenerateSamples(uint32_t t2);

	void PostFilter();

	void LogOutputChange(uint32_t t2) const;
	void LogOutputEdges(uint32_t timeBase2, const uint32_t *edges, uint32_t n) const;

	ATScheduler *mpScheduler;
	ATPokeyTables *mpTables;
	bool mbInitMode;

	float	mHighPassAccum;
	float	mOutputLevel;
	uint32_t	mLastFlushTime;
	int		mExternalInput;

	bool	mbSpeakerState;

	// Noise/tone flip-flop state for all four channels. This is the version updated by the
	// FireTimer() routines; change events are then produced to update the analogous bits 0-3
	// in the channel output mask.
	uint8_t	mNoiseFlipFlops = 0;

	// Noise/tone and high-pass flip-flop states. Bits 0-3 contain the noise flip-flop states,
	// updated by the output code from change events generated from mNoiseFlipFlops; bits 4-5
	// contain the high-pass flip-flops for ch1-2.
	uint8_t	mChannelOutputMask = 0;

	// Bits 0-3 set if ch1-4 is in volume-only mode (AUDCx bit 4 = 1).
	uint8_t	mVolumeOnlyMask = 0;

	// Bits 0-3 set if AUDCx bit 0-3 > 0. Note that this includes muting, so we cannot use
	// this for architectural state.
	uint8_t	mNonZeroVolumeMask = 0;

	uint8_t	mChannelVolume[4] {};
	uint32_t	mChannelVolMixIndex[4] {};

	struct BufferedState {
		uint8_t	mAUDC[4];
		uint8_t	mAUDCTL;
	};

	BufferedState mArchState {};
	BufferedState mRenderState {};

	// True if the channel is enabled for update or muted. This does NOT affect architectural
	// state; it must not affect whether flip-flops are updated.
	bool	mbChannelEnabled[4];

	struct DeferredEvent {
		bool	mbEnabled;

		/// Set if 16-bit linked mode is enabled; this requires tracking the
		/// high timer to know when to reset the low timer.
		bool	mbLinked;

		/// Timestamp of next lo event.
		uint32_t	mNextTime;

		/// Period of lo event in clocks.
		uint32_t	mPeriod;

		/// Timestamp of next hi event.
		uint32_t	mNextHiTime;

		/// Hi (16-bit) period in clocks.
		uint32_t	mHiPeriod;

		/// Offset from hi event to next lo event.
		uint32_t	mHiLoOffset;
	};

	DeferredEvent mDeferredEvents[4] {};

	struct ChangeEvent {
		uint32_t mTime;
		ChangeType mType;
		uint8_t mValue;
	};

	std::deque<ChangeEvent> mChangeQueue;

	struct PolyState {
		uint32_t	mInitMask = 0;

		uintptr_t mPoly4Offset = 0;
		uintptr_t mPoly5Offset = 0;
		uintptr_t mPoly9Offset = 0;
		uintptr_t mPoly17Offset = 0;

		uint32_t	mLastPoly17Time = 0;
		uint32_t	mPoly17Counter = 0;
		uint32_t	mLastPoly9Time = 0;
		uint32_t	mPoly9Counter = 0;
		uint32_t	mLastPoly5Time = 0;
		uint32_t	mPoly5Counter = 0;
		uint32_t	mLastPoly4Time = 0;
		uint32_t	mPoly4Counter = 0;

		void UpdatePoly17Counter(uint32_t t);
		void UpdatePoly9Counter(uint32_t t);
		void UpdatePoly5Counter(uint32_t t);
		void UpdatePoly4Counter(uint32_t t);
	} mPolyState;

	uint32_t	mBlockStartTime = 0;
	uint32_t	mBlockStartTime2 = 0;
	uint32_t	mOutputSampleCount = 0;
		
	ATPokeyAudioLog	*mpAudioLog = nullptr;

	// The sorted edge lists hold ordered output change events. The events are stored as
	// packed bitfields for fast merging:
	//
	//	bits 14-31 (18): half-cycle offset from beginning of flush operation
	//	bits  8-13 (6): AND mask to apply to flip/flops
	//	bits  0-5 (6): OR mask to apply to flip/flops
	//
	// Bits 4-5 of the masks are special as they apply to the high-pass flip/flops. The OR
	// mask for these bits is ANDed with the ch1/2 outputs, so they update the HP F/Fs instead
	// of setting them.
	typedef std::vector<uint32_t> SortedEdges;
	SortedEdges mSortedEdgesTemp[4];
	SortedEdges mSortedEdgesHpTemp1;
	SortedEdges mSortedEdgesHpTemp2;
	SortedEdges mSortedEdgesTemp2[2];
	SortedEdges mSortedEdges;

	// The channel edge lists hold an ordered list of timer underflow events. The ticks are
	// in system time (from ATScheduler).
	typedef std::vector<uint32_t> ChannelEdges;
	ChannelEdges mChannelEdges[4];
	uint32_t mChannelEdgeBases[4] {};

	std::vector<uint32_t> mSerialPulseTimes;
	float mSerialPulse = 0;

	enum : uint32_t {
		// 1271 samples is the max (35568 cycles/frame / 28 cycles/sample + 1). We add a little bit here
		// to round it out. We need a 16 sample holdover in order to run the FIR filter.
		kBufferSize = 1536,

		kMaxWriteIndex = kBufferSize - 16
	};

	alignas(16) float mRawOutputBuffer[kBufferSize];

	template<int activeChannel, bool T_UsePoly9>
	static const FireTimerRoutine kFireRoutines[2][16];
};

#endif	// f_AT_POKEYRENDERER_H

/*
	resample: low-latency usable and quick resampler

	copyright 2018-2023 by the mpg123 project
	licensed under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org

	initially written by Thomas Orgis

	The resampler consists of these steps:

	1. If output rate < 1/4 input rate:
	1.1 Repeat until output >= 1/4 input rate:
	1.1.1 Low pass with transition band from 1/4 to 1/2 input bandwidth.
	1.1.2 Drop samples to halve the input sample rate.
	1.2 Pre-emphasis to compensate for interpolator response.

	2. If output rate > 1/2 input rate:
	2.1 Pre-emphasis to compensate for interpolator response.
	2.2 2x oversampling (zero-stuffing).

	3. Low pass filter with >84% bandwidth to cutoff of the smaller of
	   target rate or input rate, between 1/4 and 1/2 of intermediate
	   sample rate.

	4. Interpolate to target rate.

	The filters are IIR-type. The interpolators are based on Olli Niemitalo's
	2001 paper titled 'Polynomial Interpolators for High-Quality Resampling of
	Oversampled Audio', available as download at
	http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf . It is these
	interpolators that drastically improve the quality compared to cubic spline
	interpolation.

	There are different pre-emphasis filters for the case with and without
	oversampling. At all times, the signal band before interpolation only
	extends up to 1/2 of Nyquist, enabling the use of interpolators designed
	for oversampled input. Without this spectral headroom, things would look
	much more distorted.

	The higher quality mode achieves distortion below 108 dB with a bandwith
	above 84%, while the lower quality mode weakens the final low pass to
	72 dB, and has distortion around that, with slightly better bandwidth.

	The downside of this all is serious phase distortion, but that does
	not matter much to the human ear for resampling before playback of the
	stream, as opposed to mixing resampled streams with some phase relations
	in mind. If you want to resample your instrument samples before combining
	them into music, you have the full signal at hand and can just use the
	best method regardless of latency. This here is not your resampler, then.

	The upside is that there is virtually no latency compared to FIR-based
	resamplers and it should be rather easy to use. You have a direct
	relation between input buffer and output buffer and do not have to
	think about draining internal windowing buffers. Data with one sample
	rate in, data with another sample rate out. Simple as that.

	The performance is very good compared to windowed-sinc interpolators,
	but is behind the FFT-based approach of libsoxr. You trade in massive
	latency for a performance gain, there. Vectorization of the code does
	not seem to bring much. A test with SSE intrinsics yielded about 10%
	speedup for single-channel operation. This is a basic problem of recursive
	computations. There is not that much dumb parallelism --- and the SIMD
	parallelism offered in current CPUs is rather dumb and useless without
	the specific data flow to be efficient. Or maybe I am just doing it
	wrong. Perhaps many channels may benefit from SIMD.

	If you want to optimize: Most time is spent in the low pass filtering. The
	interpolation itself is rather cheap. The decimation filter is also worth
	considering. Currently, it is not even optimized for efficient coefficient
	access (that's a TODO), let alone using the more efficient Direct Form II,
	or anything more fancy.

TODO: efficient ringbuffer access for the decimation filter
TODO: initialize with first sample or zero? is there an actual benefit? impulse
      response?
*/

#define NO_GROW_BUF
#define NO_SMAX
#define NO_SMIN
// SSIZE_MAX is not standard C:-/
#define _POSIX_C_SOURCE 200809L
#include "syn123_int.h"
#include "../common/debug.h"

// coefficient tables generated from coeff-ellip-6-0.01-36-16.txd
// (coeff-{filter_type}-{order}-{passband_ripple}-{rejection_dB}-{points})

// static const unsigned int lpf_db = 36;

// Order of the lowpass filter.
// You'd wish to have that as a multiple of 4 to help vectorization, but
// even 12 is too small to have real benefit from SSE (let alone wider stuff),
// and 6 seems to be a limit for stability. The filter needs to be of some minimal
// order to be steep enough and becomes unstable, especially with interpolated
// coefficients, with higher orders. This is a value I settled on after a long
// and painful process. This works between 1/4 and 1/2 of Nyquist. I used to
// believe that this can be extended down to 1/16 or even less, but this is
// noisy at best.
#define LPF_ORDER 6

// points in filter coefficient interpolation tables
#define LPF_POINTS 16

// filter critical frequency in fractions of Nyquist
static const float lpf_w_c[LPF_POINTS] = 
{
	+1.9500000000e-01f
,	+1.9867850841e-01f
,	+2.0682630822e-01f
,	+2.1914822332e-01f
,	+2.3516664873e-01f
,	+2.5424242424e-01f
,	+2.7560276877e-01f
,	+2.9837505460e-01f
,	+3.2162494540e-01f
,	+3.4439723123e-01f
,	+3.6575757576e-01f
,	+3.8483335127e-01f
,	+4.0085177668e-01f
,	+4.1317369178e-01f
,	+4.2132149159e-01f
,	+4.2500000000e-01f
};

// actual cutoff frequency in fractions of Nyquist
static const float lpf_cutoff[LPF_POINTS] = 
{
	+2.4618202393e-01f
,	+2.5064642208e-01f
,	+2.6050157602e-01f
,	+2.7531601376e-01f
,	+2.9440817636e-01f
,	+3.1688954073e-01f
,	+3.4172256063e-01f
,	+3.6778811727e-01f
,	+3.9395465344e-01f
,	+4.1914062138e-01f
,	+4.4236385073e-01f
,	+4.6277512916e-01f
,	+4.7967708029e-01f
,	+4.9253194430e-01f
,	+5.0096263453e-01f
,	+5.0475083567e-01f
};

// filter bandwidth (passband fraction in percent)
static const unsigned char lpf_bw[LPF_POINTS] = 
{
	85
,	85
,	85
,	85
,	85
,	86
,	86
,	86
,	87
,	87
,	88
,	88
,	88
,	89
,	89
,	89
};

// coefficient b_0
static const float lpf_b0[LPF_POINTS] = 
{
	+2.5060910358e-02f
,	+2.5545546150e-02f
,	+2.6658960397e-02f
,	+2.8449469363e-02f
,	+3.0975593417e-02f
,	+3.4290689410e-02f
,	+3.8423542124e-02f
,	+4.3355834055e-02f
,	+4.8998511458e-02f
,	+5.5171358029e-02f
,	+6.1592382077e-02f
,	+6.7884279531e-02f
,	+7.3603031799e-02f
,	+7.8288471817e-02f
,	+8.1529628810e-02f
,	+8.3031304399e-02f
};

// derivative d b_0/d w_c
static const float lpf_mb0[LPF_POINTS] = 
{
	+1.3023622859e-01f
,	+1.3326373417e-01f
,	+1.4006275119e-01f
,	+1.5061692736e-01f
,	+1.6489815146e-01f
,	+1.8285716898e-01f
,	+2.0437030679e-01f
,	+2.2914833420e-01f
,	+2.5663256095e-01f
,	+2.8591120057e-01f
,	+3.1568709259e-01f
,	+3.4431969261e-01f
,	+3.6995119359e-01f
,	+3.9070858956e-01f
,	+4.0495210591e-01f
,	+4.1152140765e-01f
};

// coefficients b_1 to b_n
static const float lpf_b[LPF_POINTS][LPF_ORDER] = 
{
	{ -5.1762847048e-02f, +8.4153207389e-02f, -8.3511150073e-02f
	, +8.4153207389e-02f, -5.1762847048e-02f, +2.5060910358e-02f }
,	{ -5.0602124846e-02f, +8.2431045673e-02f, -8.0086433599e-02f
	, +8.2431045673e-02f, -5.0602124846e-02f, +2.5545546150e-02f }
,	{ -4.7865340043e-02f, +7.8920478243e-02f, -7.2561589822e-02f
	, +7.8920478243e-02f, -4.7865340043e-02f, +2.6658960397e-02f }
,	{ -4.3255354022e-02f, +7.4480386867e-02f, -6.1251009130e-02f
	, +7.4480386867e-02f, -4.3255354022e-02f, +2.8449469363e-02f }
,	{ -3.6312512281e-02f, +7.0487561023e-02f, -4.6392636636e-02f
	, +7.0487561023e-02f, -3.6312512281e-02f, +3.0975593417e-02f }
,	{ -2.6445440921e-02f, +6.8811704409e-02f, -2.7845395780e-02f
	, +6.8811704409e-02f, -2.6445440921e-02f, +3.4290689410e-02f }
,	{ -1.3021846935e-02f, +7.1687243737e-02f, -4.8530219060e-03f
	, +7.1687243737e-02f, -1.3021846935e-02f, +3.8423542124e-02f }
,	{ +4.4706427843e-03f, +8.1414913409e-02f, +2.3941615853e-02f
	, +8.1414913409e-02f, +4.4706427843e-03f, +4.3355834055e-02f }
,	{ +2.6209675772e-02f, +9.9864368285e-02f, +6.0088123327e-02f
	, +9.9864368285e-02f, +2.6209675772e-02f, +4.8998511458e-02f }
,	{ +5.1814692315e-02f, +1.2783927823e-01f, +1.0454069691e-01f
	, +1.2783927823e-01f, +5.1814692315e-02f, +5.5171358029e-02f }
,	{ +8.0182308668e-02f, +1.6447692384e-01f, +1.5672311528e-01f
	, +1.6447692384e-01f, +8.0182308668e-02f, +6.1592382077e-02f }
,	{ +1.0945240695e-01f, +2.0692267707e-01f, +2.1383540251e-01f
	, +2.0692267707e-01f, +1.0945240695e-01f, +6.7884279531e-02f }
,	{ +1.3715527898e-01f, +2.5049018459e-01f, +2.7077975696e-01f
	, +2.5049018459e-01f, +1.3715527898e-01f, +7.3603031799e-02f }
,	{ +1.6054360691e-01f, +2.8938069305e-01f, +3.2088440740e-01f
	, +2.8938069305e-01f, +1.6054360691e-01f, +7.8288471817e-02f }
,	{ +1.7705368035e-01f, +3.1783679836e-01f, +3.5729711842e-01f
	, +3.1783679836e-01f, +1.7705368035e-01f, +8.1529628810e-02f }
,	{ +1.8478918080e-01f, +3.3142891364e-01f, +3.7464012836e-01f
	, +3.3142891364e-01f, +1.8478918080e-01f, +8.3031304399e-02f }
};

// derivatives (d b_1/d w_c) to (d b_n/d w_c)
static const float lpf_mb[LPF_POINTS][LPF_ORDER] = 
{
	{ +3.0947524511e-01f, -4.7927490698e-01f, +9.3391532013e-01f
	, -4.7927490702e-01f, +3.0947524507e-01f, +1.3023622860e-01f }
,	{ +3.2168974799e-01f, -4.5690983225e-01f, +9.2827504448e-01f
	, -4.5690983214e-01f, +3.2168974795e-01f, +1.3326373418e-01f }
,	{ +3.5049895985e-01f, -4.0402713817e-01f, +9.1976126033e-01f
	, -4.0402713816e-01f, +3.5049895984e-01f, +1.4006275120e-01f }
,	{ +3.9869096973e-01f, -3.1471026348e-01f, +9.1858562318e-01f
	, -3.1471026347e-01f, +3.9869096972e-01f, +1.5061692736e-01f }
,	{ +4.6976534041e-01f, -1.8018044797e-01f, +9.4154172198e-01f
	, -1.8018044798e-01f, +4.6976534042e-01f, +1.6489815146e-01f }
,	{ +5.6706828511e-01f, +1.0247608319e-02f, +1.0113425453e+00f
	, +1.0247608337e-02f, +5.6706828510e-01f, +1.8285716898e-01f }
,	{ +6.9280120305e-01f, +2.6709404538e-01f, +1.1535785914e+00f
	, +2.6709404540e-01f, +6.9280120305e-01f, +2.0437030678e-01f }
,	{ +8.4703144845e-01f, +5.9748112137e-01f, +1.3910603904e+00f
	, +5.9748112138e-01f, +8.4703144845e-01f, +2.2914833420e-01f }
,	{ +1.0268432839e+00f, +1.0012919713e+00f, +1.7366400230e+00f
	, +1.0012919713e+00f, +1.0268432839e+00f, +2.5663256095e-01f }
,	{ +1.2257925890e+00f, +1.4678100718e+00f, +2.1866645243e+00f
	, +1.4678100718e+00f, +1.2257925891e+00f, +2.8591120056e-01f }
,	{ +1.4338418956e+00f, +1.9740692730e+00f, +2.7173819083e+00f
	, +1.9740692730e+00f, +1.4338418956e+00f, +3.1568709258e-01f }
,	{ +1.6379144032e+00f, +2.4857766605e+00f, +3.2858109638e+00f
	, +2.4857766605e+00f, +1.6379144032e+00f, +3.4431969261e-01f }
,	{ +1.8231025322e+00f, +2.9609624523e+00f, +3.8352350558e+00f
	, +2.9609624523e+00f, +1.8231025322e+00f, +3.6995119359e-01f }
,	{ +1.9744172816e+00f, +3.3557576586e+00f, +4.3041807299e+00f
	, +3.3557576587e+00f, +1.9744172816e+00f, +3.9070858955e-01f }
,	{ +2.0788103417e+00f, +3.6311428529e+00f, +4.6368982737e+00f
	, +3.6311428526e+00f, +2.0788103416e+00f, +4.0495210591e-01f }
,	{ +2.1270907033e+00f, +3.7592676993e+00f, +4.7930965582e+00f
	, +3.7592676992e+00f, +2.1270907033e+00f, +4.1152140769e-01f }
};

// coefficients a_1 to a_n
static const float lpf_a[LPF_POINTS][LPF_ORDER] = 
{
	{ -3.9678160947e+00f, +7.1582705272e+00f, -7.3103432700e+00f
	, +4.4271625605e+00f, -1.4968822486e+00f, +2.2103607837e-01f }
,	{ -3.9217586689e+00f, +7.0245007818e+00f, -7.1375017960e+00f
	, +4.3093638600e+00f, -1.4547982662e+00f, +2.1489651918e-01f }
,	{ -3.8190188341e+00f, +6.7322820225e+00f, -6.7643407898e+00f
	, +4.0573843887e+00f, -1.3653052102e+00f, +2.0191441068e-01f }
,	{ -3.6618359076e+00f, +6.3015570638e+00f, -6.2251488099e+00f
	, +3.6990273202e+00f, -1.2392343109e+00f, +1.8379956602e-01f }
,	{ -3.4544704643e+00f, +5.7631160859e+00f, -5.5687366968e+00f
	, +3.2721548186e+00f, -1.0907981304e+00f, +1.6273969374e-01f }
,	{ -3.2034885530e+00f, +5.1559829152e+00f, -4.8508981080e+00f
	, +2.8177192974e+00f, -9.3461709876e-01f, +1.4091459119e-01f }
,	{ -2.9178775764e+00f, +4.5233648501e+00f, -4.1259259822e+00f
	, +2.3730384158e+00f, -7.8317396639e-01f, +1.2011720880e-01f }
,	{ -2.6088711387e+00f, +3.9075669790e+00f, -3.4391290636e+00f
	, +1.9669306242e+00f, -6.4531233315e-01f, +1.0156466880e-01f }
,	{ -2.2894468741e+00f, +3.3447965777e+00f, -2.8221523134e+00f
	, +1.6175400427e+00f, -5.2592289748e-01f, +8.5891269383e-02f }
,	{ -1.9735577122e+00f, +2.8610227169e+00f, -2.2920156678e+00f
	, +1.3327010249e+00f, -4.2655597907e-01f, +7.3258414200e-02f }
,	{ -1.6752255568e+00f, +2.4698591405e+00f, -1.8535096177e+00f
	, +1.1119902037e+00f, -3.4650830575e-01f, +6.3506595124e-02f }
,	{ -1.4076456238e+00f, +2.1728886190e+00f, -1.5035402243e+00f
	, +9.4944185607e-01f, -2.8395564864e-01f, +5.6296779542e-02f }
,	{ -1.1824218415e+00f, +1.9621904134e+00f, -1.2356636112e+00f
	, +8.3617447471e-01f, -2.3684730680e-01f, +5.1219220854e-02f }
,	{ -1.0089985054e+00f, +1.8243200388e+00f, -1.0434432700e+00f
	, +7.6259361700e-01f, -2.0344554024e-01f, +4.7870210629e-02f }
,	{ -8.9429544087e-01f, +1.7447560305e+00f, -9.2208898364e-01f
	, +7.2012730057e-01f, -1.8252811740e-01f, +4.5906155400e-02f }
,	{ -8.4251214238e-01f, +1.7118654093e+00f, -8.6865163772e-01f
	, +7.0252569491e-01f, -1.7335864376e-01f, +4.5082431755e-02f }
};

// derivatives (d a_1/d w_c) to (d a_n/d w_c)
static const float lpf_ma[LPF_POINTS][LPF_ORDER] = 
{
	{ +1.2492433161e+01f, -3.6517018434e+01f, +4.7357395713e+01f
	, -3.2369784989e+01f, +1.1586055408e+01f, -1.6933077254e+00f }
,	{ +1.2548734957e+01f, -3.6212194682e+01f, +4.6616582888e+01f
	, -3.1678742548e+01f, +1.1296037667e+01f, -1.6449911143e+00f }
,	{ +1.2669371015e+01f, -3.5511519947e+01f, +4.4983356850e+01f
	, -3.0180967586e+01f, +1.0676583147e+01f, -1.5427341805e+00f }
,	{ +1.2841272347e+01f, -3.4388021134e+01f, +4.2540425267e+01f
	, -2.8002157377e+01f, +9.7976496751e+00f, -1.3997892255e+00f }
,	{ +1.3046121049e+01f, -3.2820302039e+01f, +3.9430990541e+01f
	, -2.5324586959e+01f, +8.7534918962e+00f, -1.2330749768e+00f }
,	{ +1.3263302462e+01f, -3.0810041034e+01f, +3.5857000371e+01f
	, -2.2361001245e+01f, +7.6447176195e+00f, -1.0593731098e+00f }
,	{ +1.3473094693e+01f, -2.8396265557e+01f, +3.2063474406e+01f
	, -1.9323493868e+01f, +6.5616850615e+00f, -8.9237379829e-01f }
,	{ +1.3659446627e+01f, -2.5661488138e+01f, +2.8308712196e+01f
	, -1.6394785270e+01f, +5.5731925958e+00f, -7.4123740640e-01f }
,	{ +1.3811818531e+01f, -2.2727774254e+01f, +2.4827395931e+01f
	, -1.3709404604e+01f, +4.7220289931e+00f, -6.1062450746e-01f }
,	{ +1.3925825330e+01f, -1.9744316114e+01f, +2.1797535816e+01f
	, -1.1348774538e+01f, +4.0264909835e+00f, -5.0168575497e-01f }
,	{ +1.4002735155e+01f, -1.6870564740e+01f, +1.9320924822e+01f
	, -9.3489487064e+00f, +3.4855436350e+00f, -4.1340259191e-01f }
,	{ +1.4048120627e+01f, -1.4259541011e+01f, +1.7421558040e+01f
	, -7.7156336592e+00f, +3.0852238544e+00f, -3.4381636627e-01f }
,	{ +1.4070068384e+01f, -1.2044779182e+01f, +1.6060419989e+01f
	, -6.4399486362e+00f, +2.8047048140e+00f, -2.9090554179e-01f }
,	{ +1.4077328201e+01f, -1.0332356944e+01f, +1.5160803823e+01f
	, -5.5100288590e+00f, +2.6213968366e+00f, -2.5305663714e-01f }
,	{ +1.4077683041e+01f, -9.1976058009e+00f, +1.4636862941e+01f
	, -4.9166414668e+00f, +2.5150332199e+00f, -2.2919336158e-01f }
,	{ +1.4076709161e+01f, -8.6849570278e+00f, +1.4418874781e+01f
	, -4.6538327513e+00f, +2.4707840042e+00f, -2.1869079956e-01f }
};

// Low pass with 1/4 pass band, 1/4 transition band on top, for
// instant decimation by 2 and further processing to a rate below
// 1/4 of original sampling rate.
// This tuned instance needs only order 12 once and gives more
// attenuation than the regular resampling lowpass filter. Also, the
// pass band is maximaly preserved for no additional bandwidth loss.
// It is an Chebychev type 2 filter with clean pass band but of course
// wild phase shifts. Designed with octave's cheby2(12,112,0.5).

#define LPF_4_ORDER 12

static const float lpf_4_coeff[2][LPF_4_ORDER+1] =
{
	{ +4.942274456389e-04f, +2.456588519653e-03f, +7.333608109998e-03f
	, +1.564190404912e-02f, +2.595749312226e-02f, +3.475411633752e-02f
	, +3.823783863736e-02f, +3.475411633752e-02f, +2.595749312226e-02f
	, +1.564190404912e-02f, +7.333608109998e-03f, +2.456588519652e-03f
	, +4.942274456389e-04f }
,	{ +1.000000000000e+00f, -3.414853772012e+00f, +6.812049477837e+00f
	, -8.988639858221e+00f, +8.737226725937e+00f, -6.382745001626e+00f
	, +3.590542256812e+00f, -1.543464216385e+00f, +5.036659057430e-01f
	, -1.203614775500e-01f, +2.006360736124e-02f, -2.070940463771e-03f
	, +1.010063725617e-04f }
};

// 4th-order pre-emphasis filters for the differing interpolations

#define PREEMP_ORDER 5

static const float preemp_1xopt4p4o_coeff[2][6] =
{
	{ +1.4537908355e+00f, -1.5878441906e-01f, +7.6588945557e-01f
	, -1.1187528142e-01f, +4.9169512092e-02f, -6.1692905372e-03f }
,	{ +1.0000000000e+00f, +1.9208553603e-01f, +6.4223479412e-01f
	, +8.4340248522e-02f, +6.9072561856e-02f, +4.2876716239e-03f }
};
static const float preemp_1xopt6p5o_coeff[2][6] =
{
	{ +1.7736168976e+00f, -2.9739831101e-01f, +8.3183026784e-01f
	, -1.5434578823e-01f, +3.0236127398e-02f, -2.1054630699e-03f }
,	{ +1.0000000000e+00f, +2.9206686404e-01f, +6.5605633523e-01f
	, +1.4922672324e-01f, +7.4792388477e-02f, +9.6914195316e-03f }
};
static const float preemp_2xopt4p4o_coeff[2][6] =
{
	{ +1.1866573548e+00f, +1.1973031478e+00f, -9.4241923873e-02f
	, -4.0863707262e-01f, -8.0536389300e-02f, -3.8911856607e-04f }
,	{ +1.0000000000e+00f, +1.2122990222e+00f, +1.0444255630e-01f
	, -3.7674911658e-01f, -1.3106237366e-01f, -8.7740900285e-03f }
};
static const float preemp_2xopt6p5o_coeff[2][6] =
{
	{ +1.2995411641e+00f, +1.3148944902e+00f, -4.3762040619e-02f
	, -3.7595940266e-01f, -6.2452466903e-02f, +8.5987740998e-04f }
,	{ +1.0000000000e+00f, +1.3227482221e+00f, +2.6592565819e-01f
	, -3.1166281821e-01f, -1.3266510256e-01f, -1.1224337963e-02f }
};

// TODO: switch the decimation filter to Direct Form II
struct lpf4_hist
{
	// Filter history (to be switched to Direct Form 2).
	float x[LPF_4_ORDER];
	float y[LPF_4_ORDER];
};

#ifndef NO_DE_DENORM
// Battling denormals. They naturally occur in IIR filters and
// need to be avoided since they hurt performance on many CPUs.
// Usually, gcc -ffast-math would also avoid them, but we cannot
// count on that and debugging performance issues becomes confusing.
// The idea is to add an alternatingly positive/negative small number
// that is orders of magnitude above denormals to freshly computed
// samples.
// Downside: You never see a true zero if ther is true zero on input.
// Upside would be that you'd convert to 24 or 32 bit integer anyway,
// where you'd get your zero back (1 in 32 bit is around 5e-10.
static const float denorm_base = 1e-15f;
#define DE_DENORM(val, base) (val) += (base);
#define DE_DENORM_FLIP(base) (base) = -(base);
#define DE_DENORM_INIT(base, sign) (base) = (sign)*denorm_base;
#else
#define DE_DENORM(val, base)
#define DE_DENORM_FLIP(base)
#define DE_DENORM_INIT(base, sign)
#endif

struct decimator_state
{
	unsigned int sflags; // also stores decimation position (0 or 1)
	unsigned int n1;
	struct lpf4_hist *ch; // one for each channel, pointer into common array
	float *out_hist; // channels*STAGE_HISTORY, pointer to common block
#ifndef NO_DE_DENORM
	float dede; // current de-denormalization offset
#endif
};

#ifdef SYN123_HIGH_PREC
typedef double lpf_sum_type;
#else
typedef float lpf_sum_type;
#endif

// The amount of lowpass runs determines much of the quality.
// Each run gives lpf_db reduction. The lowpass also includes the
// pre-emphasis and oversampling.
#define LPF_TIMES 3
#define DIRTY_LPF_TIMES 2
#define LOWPASS                 lowpass3_df2
#define LOWPASS_PREEMP          lowpass3_df2_preemp
#define LOWPASS_PREEMP_2X       lowpass3_df2_preemp_2x
#define DIRTY_LOWPASS           lowpass2_df2
#define DIRTY_LOWPASS_PREEMP    lowpass2_df2_preemp
#define DIRTY_LOWPASS_PREEMP_2X lowpass2_df2_preemp_2x
// The number of points used in the interpolators.
#define FINE_POINTS 6
#define DIRTY_POINTS 4

// The data structure accomodates a fixed amount of low pass applications.
#define LPF_MAX_TIMES LPF_TIMES

// Let's define what we see a a reasonable history size for an IIR lowpass.
// On arrival of a new sample, <order> previous samples are needed, both
// input and output (recursive). Let's simply double that to cater for
// the recursive nature, ensuring that each recursive input at least already
// contains something from the new samples
#define LPF_HISTORY(order) (2*(order))

// Number of samples to keep for smooth rate changes on
// addition/removal of stages.
#define STAGE_HISTORY 48

enum state_flags
{
	inter_flow   = 1<<0
,	preemp_configured = 1<<1
,	preemp_flow = 1<<2
,	decimate_store = 1<<3 // one skipped sample from last buffer
,	oversample_2x = 1<<4 // x2 oversampling or not
,	lowpass_configured = 1<<5
,	lowpass_flow = 1<<6
,	dirty_method = 1<<7
,	smooth_change = 1<<8
};

// TODO: tune that to the smallest sensible value.
#ifndef SYN123_BATCH
#define BATCH 128
#else
#define BATCH SYN123_BATCH
#endif

#if (BATCH < STAGE_HISTORY)
#error "Batch size cannot be smaller than the stage history."
#endif

struct channel_history
{
	// Direct Form II histories for preemp and lowpass
	float pre_w[PREEMP_ORDER]; // history for Direct Form II
	float lpf_w[LPF_MAX_TIMES][LPF_ORDER];
	float x[5]; // past input samples for interpolator
	float c[6]; // interpolator coefficients
};

struct resample_data
{
	unsigned int sflags;
	// The configured resampling function to call.
	// Decides: oversampling/nothing/decimation, fine/dirty
	size_t (*resample_func)(struct resample_data *, float *, size_t, float *);
	// The lowpass functions used before interpolation, including preemp.
	void (*lowpass_2x_func)(struct resample_data *, float *, size_t, float *);
	void (*lowpass_func)   (struct resample_data *, float *, size_t);
	// Maximum number of input samples that can be fed in one go to have the
	// number of output samples safely within the limit of size_t.
	size_t input_limit;
	// Decimator state. Each stage has a an instance of lpf_4 and the decimator.
	unsigned int decim_stages;
	struct decimator_state *decim;
	// storage for the above to avoid nested malloc
	struct lpf4_hist *decim_hist;
	struct channel_history *ch;
	// Blocks of STAGE_HISTORY*channels samples.
	// 0: plain input for first decimation, oversampling or direct interpolation
	// 1: output of first decimation
	// 2: output of second decimation, etc.
	float *stage_history; // storage for past input of final (2x) lowpass
	float *frame;  // One PCM frame to work on (one sample for each channel).
	float *prebuf; // [channels*BATCH]
	float *upbuf;  // [channels*2*BATCH]
#ifndef NO_DE_DENORM
	float dede; // de-denormalization offset, alternatingly used both in preemp and lpf
#endif
	// Final lowpass filter setup.
	float lpf_cutoff;
	float lpf_w_c;
	unsigned char lpf_bw; // Bandwidth in percent, rounded to integer.
	// Pre-emphasis filter coefficients and history.
	float pre_b0;
	unsigned char pre_n1;
	float pre_b[PREEMP_ORDER][PREEMP_ORDER];
	float pre_a[PREEMP_ORDER][PREEMP_ORDER];
	// Same for the low pass.
	float lpf_b0;
	// Low pass filter history in ringbuffer. For each ringbuffer state, there is
	// one mirroring set of coefficients to avoid re-sorting during application.
	unsigned char lpf_n1; // Current index of the most recent past.
	// It may help to have these aligned, but since LPF_ORDER is not 4 nor 8, things
	// just don't add up for vector instructions.
	float lpf_b[LPF_ORDER][LPF_ORDER];
	float lpf_a[LPF_ORDER][LPF_ORDER];
	// Interpolator state.
	// In future, I could pull the same trick with ringbuffer and copies
	// of coefficients (matrices, even) as for lpf.
	// TODO: maybe store more of these for better filter adaption on rate changes, or
	// at least always the full set of 5.
	long offset; // interpolator offset
	unsigned int channels;
	long inrate;
	long vinrate;
	long outrate;
	long voutrate;
};

// Interpolation of low pass coefficients for specified cutoff, using
// the precomputed table. Instead of properly computing the filter
// at runtime, a cheap spline interpolation in the stored tables
// does the job, too. This works for filters that are not too unstable.
// The employed 6th-order filter still works with the associated rounding
// errors, in the limited range of cutoff between 1/4 and 1/2 of Nyquist.

// Constructing the low pass filter for a bit less than claimed cutoff,
// numerics shift things a bit.
static const float lpf_cut_scale = 0.995f;

// Reference implementation of the cubic spline evaluation.
// x: time coordinate to evaluate at
// xa: lower interval boundary
// xb: upper interval boundary
// a: value at xa
// b: value at xb
// da: derivative at xa
// db: derivative at db
static float spline_val( float x, float xa, float xb
,	float a, float b, float da, float db )
{
	float d = xb-xa;
	float t = (x-xa)/d;
	float t2 = t*t;
	float t3 = t2*t;
	float h00 = 2*t3-3*t2+1;
	float h10 = -2*t3+3*t2;
	float h01d = (t3-2*t2+t)*d;
	float h11d = (t3-t2)*d;
	return h00*a + h10*b + h01d*da + h11d*db;
}

// Find coordiante x in interpolation table val.
// Returns index for the lower boundary. Monotonic increase
// is assumed.
static unsigned int lpf_entry(const float *val, float x)
{
	unsigned int i = 0;
	while(i<(LPF_POINTS-2) && val[i+1] <= x)
		++i;
	return i;
}

// Derivative of value with respect to w_c at given
// index. Intervals are not of equal width ... so central
// difference is not straightforward. I'll simply use a
// inverse-weighted mean of both sides.
static float lpf_deriv( const float x[LPF_POINTS]
,	const float val[LPF_POINTS], unsigned int i )
{
	float w = 0;
	float m = 0;
	// lpf_w_c assumed to be monotonically increasing
	if(i<LPF_POINTS-1)
	{
		float w1 = x[i+1]-x[i];
		m += (val[i+1]-val[i])/(w1*w1);
		w += 1.f/w1; 
	}
	if(i>0)
	{
		float w0 = x[i]-x[i-1];
		m += (val[i]-val[i-1])/(w0*w0);
		w += 1.f/w0;
	}
	return m/w; 
}

// Static ring buffer index used for filter history and coefficients.
// i: offset
// n1: current position of the first entry
// s: size of the ring buffer
#define RING_INDEX(i, n1, s) ( ((n1)+(i)) % s )

// Compute actual low pass coefficients for the cutoff in the handle.
// This interpolates in the tables without subsequent scaling for zero
// DC gain, which has been shown not to be useful.
static void lpf_init(struct resample_data *rd)
{
	rd->sflags &= ~lowpass_configured;
	float real_cutoff = rd->lpf_cutoff * lpf_cut_scale;
	// It does not make sense to return an error here. It is an internal
	// implementation error if the table is too small. Just run with the
	// best value we have.
	if(real_cutoff < lpf_cutoff[0])
		real_cutoff = lpf_cutoff[0];
	if(real_cutoff > lpf_cutoff[LPF_POINTS-1])
		real_cutoff = lpf_cutoff[LPF_POINTS-1];
	// The coefficient interpolation needs w_c != cutoff.
	unsigned int lpfi = lpf_entry(lpf_cutoff, real_cutoff);
	rd->lpf_w_c = spline_val
	(
		real_cutoff, lpf_cutoff[lpfi], lpf_cutoff[lpfi+1]
	,	lpf_w_c[lpfi], lpf_w_c[lpfi+1]
	,	lpf_deriv(lpf_cutoff, lpf_w_c, lpfi)
	,	lpf_deriv(lpf_cutoff, lpf_w_c, lpfi+1)
	);
	mdebug("lowpass cutoff %g; w_c %g", rd->lpf_cutoff, rd->lpf_w_c);
	lpfi = lpf_entry(lpf_w_c, rd->lpf_w_c);
	rd->lpf_bw = (unsigned char)( 0.5 + lpf_cut_scale*(lpf_bw[lpfi]
	+	(rd->lpf_w_c-lpf_w_c[lpfi])/(lpf_w_c[lpfi+1]-lpf_w_c[lpfi])
	*	(lpf_bw[lpfi+1]-lpf_bw[lpfi])) );
	mdebug("lowpass bandwidth: %u%%", rd->lpf_bw);
	rd->lpf_b0 = spline_val( rd->lpf_w_c, lpf_w_c[lpfi], lpf_w_c[lpfi+1]
	,	lpf_b0[lpfi], lpf_b0[lpfi+1]
	,	lpf_mb0[lpfi], lpf_mb0[lpfi+1] );
	float b[LPF_ORDER];
	float a[LPF_ORDER];
	for(int i=0; i<LPF_ORDER; ++i)
	{
		b[i] = spline_val( rd->lpf_w_c
		,	lpf_w_c[lpfi], lpf_w_c[lpfi+1]
		,	lpf_b[lpfi][i], lpf_b[lpfi+1][i]
		,	lpf_mb[lpfi][i], lpf_mb[lpfi+1][i] );
		a[i] = spline_val( rd->lpf_w_c
		,	lpf_w_c[lpfi], lpf_w_c[lpfi+1]
		,	lpf_a[lpfi][i], lpf_a[lpfi+1][i]
		,	lpf_ma[lpfi][i], lpf_ma[lpfi+1][i] );
	}
	memset(rd->lpf_a, 0, sizeof(float)*LPF_ORDER*LPF_ORDER);
	memset(rd->lpf_b, 0, sizeof(float)*LPF_ORDER*LPF_ORDER);
	for(int i=0; i<LPF_ORDER; ++i)
	{
		for(int j=0; j<LPF_ORDER; ++j)
		{
			rd->lpf_b[j][RING_INDEX(i,j,LPF_ORDER)] = b[i];
			rd->lpf_a[j][RING_INDEX(i,j,LPF_ORDER)] = a[i];
		}
	}
	debug("lpf coefficients [b,a]:");
	mdebug("%.15e", rd->lpf_b0);
	for(int i=0; i<LPF_ORDER; ++i)
		mdebug("%.15e", rd->lpf_b[0][i]);
	mdebug("%.15e", 1.);
	for(int i=0; i<LPF_ORDER; ++i)
		mdebug("%.15e", rd->lpf_a[0][i]);
	rd->sflags |= lowpass_configured;
}

// Compute an estimate of the magnitude of lowpass history values.
// This is used to re-scale those values on sample ratio changes to
// try to keep a smooth ride. Relate this to df2_initval().
// First approach: b_0 / (1+sum(a))
static float lpf_history_scale(struct resample_data *rd)
{
	float asum = 1;
	for(unsigned int i=0; i<LPF_ORDER; ++i)
		asum += rd->lpf_a[0][i];
	return rd->lpf_b0/asum;
}

static void preemp_init(struct resample_data *rd)
{
	const float* pre_b;
	const float* pre_a;
	rd->sflags &= ~preemp_configured;

	#define PREEMP_COEFF(name) \
		pre_b     = preemp_ ## name ## _coeff[0]; \
		pre_a     = preemp_ ## name ## _coeff[1];
	if(rd->sflags & oversample_2x)
	{
		if(rd->sflags & dirty_method)
		{
			PREEMP_COEFF(2xopt4p4o);
		} else
		{
			PREEMP_COEFF(2xopt6p5o);
		}
	}
	else
	{
		if(rd->sflags & dirty_method)
		{
			PREEMP_COEFF(1xopt4p4o);
		} else
		{
			PREEMP_COEFF(1xopt6p5o);
		}
	}

	rd->pre_b0 = pre_b[0];
	for(int i=0; i<PREEMP_ORDER; ++i)
	{
		for(int j=0; j<PREEMP_ORDER; ++j)
		{
			rd->pre_b[j][RING_INDEX(i,j,PREEMP_ORDER)] = pre_b[i+1];
			rd->pre_a[j][RING_INDEX(i,j,PREEMP_ORDER)] = pre_a[i+1];
		}
	}

	rd->sflags |= preemp_configured;
}


// Store the last samples of given input data in the respective
// stage history buffer.
static void stage_history( struct resample_data *rd
,	unsigned int stage, float *in, size_t ins )
{
	if(!rd->stage_history)
		return;
	// Case 1: We got enough and can fill it all.
	// Case 2: Just a bit, have to shift existing
	//         history to append to.
	float *hist = rd->stage_history + stage*STAGE_HISTORY*rd->channels;
	if(ins >= STAGE_HISTORY)
		memcpy( hist, in+(ins-STAGE_HISTORY)*rd->channels
		,	STAGE_HISTORY*rd->channels*sizeof(float) );
	else
	{
		memmove( hist, hist+ins*rd->channels
		,	(STAGE_HISTORY-ins)*rd->channels*sizeof(float) );
		memcpy( hist+(STAGE_HISTORY-ins)*rd->channels, in
		,	ins*rd->channels*sizeof(float) );
	}
}


static void stage_history_init( struct resample_data *rd
,	unsigned int stage, float *in )
{
	if(!rd->stage_history)
		return;
	float *hist = rd->stage_history + stage*STAGE_HISTORY*rd->channels;
	for(size_t i=0; i<STAGE_HISTORY; ++i)
		for(unsigned int c=0; c<rd->channels; ++c)
			hist[i*rd->channels+c] = in[c];
}

// Actual resampling workers.

// To make things easier to compilers, things like the number of times
// a low pass filter is repeatedly applied are hardcoded in functions
// generated with a set of macros to avoid code duplication.

// One decimation step: low pass with transition band from 1/2 to 1/4,
// drop every second sample. This works within the input buffer.
static size_t decimate( struct resample_data *rrd, unsigned int dstage
,	float *in, size_t ins )
{
	struct decimator_state *rd = rrd->decim+dstage;
	if(!ins)
		return 0;
	mdebug("decimating %zu samples", ins);
	if(!(rd->sflags & lowpass_flow))
	{
		for(unsigned int c=0; c<rrd->channels; ++c)
			for(unsigned int j=0; j<LPF_4_ORDER; ++j)
				rd->ch[c].x[j] = rd->ch[c].y[j] = in[c];
		rd->n1 = 0;
		rd->sflags |= lowpass_flow|decimate_store;
		DE_DENORM_INIT(rd->dede, dstage % 2 ? -1 : +1)
		stage_history_init(rrd, dstage+1, in);
	}
	float *out  = in; // output worker
	float *oout = in; // for history storage
	size_t outs = 0;
#ifndef SYN123_NO_CASES
	switch(rrd->channels)
	{
		case 1: for(size_t i=0; i<ins; ++i)
		{
			int ni[LPF_4_ORDER];
			for(int j=0; j<LPF_4_ORDER; ++j)
				ni[j] = RING_INDEX(j, rd->n1, LPF_4_ORDER);
			rd->n1 = ni[LPF_4_ORDER-1];
			// y0 = b0x0 + b1x1 + ... + bxyn - a1y1 - ... - anyn
			// Switching y to double pushes roundoff hf noise down (?).
			lpf_sum_type ny = lpf_4_coeff[0][0] * in[i]; // a0 == 1 implicit here
			for(int j=0; j<LPF_4_ORDER; ++j)
			{
				// This direct summing without intermediate asum and bsum
				// pushs roundoff hf noise down.
				ny += rd->ch[0].x[ni[j]]*lpf_4_coeff[0][j+1];
				ny -= rd->ch[0].y[ni[j]]*lpf_4_coeff[1][j+1];
			}
			DE_DENORM(ny, rd->dede)
			rd->ch[0].x[rd->n1] = in[i];
			rd->ch[0].y[rd->n1] = ny;
			// Drop every second sample.
			// Maybe its faster doing this in a separate step after all?
			if(rd->sflags & decimate_store)
			{
				DE_DENORM_FLIP(rd->dede)
				*(out++) = ny;
				rd->sflags &= ~decimate_store;
				++outs;
			} else
				rd->sflags |= decimate_store;
		}
		break;
		case 2: for(size_t i=0; i<ins; ++i)
		{
			int ni[LPF_4_ORDER];
			for(int j=0; j<LPF_4_ORDER; ++j)
				ni[j] = RING_INDEX(j, rd->n1, LPF_4_ORDER);
			rd->n1 = ni[LPF_4_ORDER-1];
			float frame[2];
			for(unsigned int c=0; c<2; ++c)
			{
				// y0 = b0x0 + b1x1 + ... + bxyn - a1y1 - ... - anyn
				// Switching y to double pushes roundoff hf noise down (?).
				lpf_sum_type ny = lpf_4_coeff[0][0] * in[c]; // a0 == 1 implicit here
				for(int j=0; j<LPF_4_ORDER; ++j)
				{
					// This direct summing without intermediate asum and bsum
					// pushs roundoff hf noise down.
					ny += rd->ch[c].x[ni[j]]*lpf_4_coeff[0][j+1];
					ny -= rd->ch[c].y[ni[j]]*lpf_4_coeff[1][j+1];
				}
				DE_DENORM(ny, rd->dede)
				rd->ch[c].x[rd->n1] = in[c];
				rd->ch[c].y[rd->n1] = ny;
				frame[c] = ny;
			}
			in += 2;
			// Drop every second sample.
			// Maybe its faster doing this in a separate step after all?
			if(rd->sflags & decimate_store)
			{
				DE_DENORM_FLIP(rd->dede)
				*(out++) = frame[0];
				*(out++) = frame[1];
				rd->sflags &= ~decimate_store;
				++outs;
			} else
				rd->sflags |= decimate_store;
		}
		break;
		default:
#endif
		for(size_t i=0; i<ins; ++i)
		{
			int ni[LPF_4_ORDER];
			for(int j=0; j<LPF_4_ORDER; ++j)
				ni[j] = RING_INDEX(j, rd->n1, LPF_4_ORDER);
			rd->n1 = ni[LPF_4_ORDER-1];
			for(unsigned int c=0; c<rrd->channels; ++c)
			{
				// y0 = b0x0 + b1x1 + ... + bxyn - a1y1 - ... - anyn
				// Switching y to double pushes roundoff hf noise down (?).
				lpf_sum_type ny = lpf_4_coeff[0][0] * in[c]; // a0 == 1 implicit here
				for(int j=0; j<LPF_4_ORDER; ++j)
				{
					// This direct summing without intermediate asum and bsum
					// pushs roundoff hf noise down.
					ny += rd->ch[c].x[ni[j]]*lpf_4_coeff[0][j+1];
					ny -= rd->ch[c].y[ni[j]]*lpf_4_coeff[1][j+1];
				}
				DE_DENORM(ny, rd->dede)
				rd->ch[c].x[rd->n1] = in[c];
				rd->ch[c].y[rd->n1] = ny;
				rrd->frame[c] = ny;
			}
			in += rrd->channels;
			// Drop every second sample.
			// Maybe its faster doing this in a separate step after all?
			if(rd->sflags & decimate_store)
			{
				DE_DENORM_FLIP(rd->dede)
				for(unsigned int c=0; c<rrd->channels; ++c)
					*(out++) = rrd->frame[c];
				rd->sflags &= ~decimate_store;
				++outs;
			} else
				rd->sflags |= decimate_store;
		}
#ifndef SYN123_NO_CASES
	}
#endif
	stage_history(rrd, dstage+1, oout, outs);
	return outs;
}

// Initialization of Direct Form 2 history.
// Return a constant value for w[n] that represents an endless history of given insample.
// Given the recursive definition: w[n] = x[n] - sum_i(a[i] * w[n-i])
// Given my ansatz of endless stream of x[n-i] = x[n] and hence w[n-i] = w[n], it follows
// that
//       w[n] = x[n] - sum_i(a[i] * w[n]) = x[n] - sum_i(a[i]) * w[n]
//  <=>  (1 + sum_i(a[i])) w[n] = x[n]
//  <=>  w[n] = 1 / (1 + sum_i(a[i])) * x[n]
// Looks simple. Just a scale value.
static float df2_initval(unsigned int order, float *filter_a, float insample)
{
	float asum = 1.;
	for(unsigned int i=0; i<order; ++i)
		asum += filter_a[i];
	float scale = 1.f/asum;
	return scale * insample;
}

#define PREEMP_DF2_BEGIN(initval, channels, ret) \
	if(!(rd->sflags & preemp_flow)) \
	{ \
		if(!(rd->sflags & preemp_configured)) \
		{ \
			fprintf(stderr, "unconfigured pre-emphasis\n"); \
			return ret; \
		} \
		for(unsigned int c=0; c<channels; ++c) \
		{ \
			float iv = df2_initval(PREEMP_ORDER, rd->pre_a[0], initval[c]); \
			for(int i=0; i<PREEMP_ORDER; ++i) \
				rd->ch[c].pre_w[i] = iv; \
		} \
		rd->pre_n1 = 0; \
		rd->sflags |= preemp_flow; \
	}

// Might not be faster than DFI, but for sure needs less storage.
#define PREEMP_DF2_SAMPLE(in, out, channels) \
	{ \
		unsigned char n1 = rd->pre_n1; \
		rd->pre_n1 = RING_INDEX(PREEMP_ORDER-1, rd->pre_n1, PREEMP_ORDER); \
		for(unsigned int c=0; c<channels; ++c) \
		{ \
			lpf_sum_type ny = 0; \
			lpf_sum_type nw = 0; \
			for(unsigned char i=0; i<PREEMP_ORDER; ++i) \
			{ \
				ny += rd->ch[c].pre_w[i]*rd->pre_b[n1][i]; \
				nw -= rd->ch[c].pre_w[i]*rd->pre_a[n1][i]; \
			} \
			nw += in[c]; \
			DE_DENORM(nw, rd->dede) \
			ny += rd->pre_b0 * nw; \
			rd->ch[c].pre_w[rd->pre_n1] = nw; \
			out[c] = ny; \
		} \
	}

// Beginning of a low pass function with on-the-fly initialization
// of filter history.
// For oversampling lowpass with zero-stuffing, initialize to half of first
// input sample.
#define LPF_DF2_BEGIN(times,initval,channels,ret) \
	if(!(rd->sflags & lowpass_flow)) \
	{ \
		if(!(rd->sflags & lowpass_configured)) \
		{ \
			fprintf(stderr, "unconfigured lowpass\n"); \
			return ret; \
		} \
		rd->lpf_n1 = 0; \
		for(unsigned int c=0; c<channels; ++c) \
		{ \
			float iv = df2_initval(LPF_ORDER, rd->lpf_a[0], initval[c]); \
			for(int j=0; j<times; ++j) \
				for(int i=0; i<LPF_ORDER; ++i) \
					rd->ch[c].lpf_w[j][i] = iv; \
		} \
		rd->sflags |= lowpass_flow; \
	} \
	int n1 = rd->lpf_n1; \
	int n1n;

#define LPF_DF2_SAMPLE(times, in, out, channels) \
	n1n = RING_INDEX(LPF_ORDER-1, n1, LPF_ORDER); \
	for(unsigned int c=0; c<channels; ++c) \
	{ \
		float old_y = in[c]; \
		for(int j=0; j<times; ++j) \
		{ \
			lpf_sum_type w = old_y; \
			lpf_sum_type y = 0; \
			for(int k=0; k<LPF_ORDER; ++k) \
			{ \
				y += rd->ch[c].lpf_w[j][k]*rd->lpf_b[n1][k]; \
				w -= rd->ch[c].lpf_w[j][k]*rd->lpf_a[n1][k]; \
			} \
			DE_DENORM(w, rd->dede) \
			rd->ch[c].lpf_w[j][n1n] = w; \
			y += rd->lpf_b0*w; \
			old_y = y; \
		} \
		out[c] = old_y; \
	} \
	n1 = n1n;

#define LPF_DF2_END \
	rd->lpf_n1 = n1;

// Define normal and oversampling low pass with pre-emphasis in direct form 2.
// The parameter determines the number of repeated applications of the same
// low pass.

#ifndef SYN123_NO_CASES
#define LOWPASS_DF2_FUNCSX(times) \
\
static void lowpass##times##_df2_preemp_2x(struct resample_data *rd, float *in, size_t ins, float *out) \
{ \
	if(!ins) \
		return; \
	LPF_DF2_BEGIN(times,in,rd->channels,) \
	PREEMP_DF2_BEGIN(in,rd->channels,); \
	switch(rd->channels) \
	{ \
		case 1: for(size_t i=0; i<ins; ++i) \
		{ \
			float sample; \
			PREEMP_DF2_SAMPLE(in, (&sample), 1) \
			/* Zero-stuffing! Insert zero after making up for energy loss. */ \
			sample *= 2; \
			LPF_DF2_SAMPLE(times, (&sample), out, 1) \
			out++; \
			sample = 0; \
			LPF_DF2_SAMPLE(times, (&sample), out, 1) \
			DE_DENORM_FLIP(rd->dede) \
			out++; \
			in++; \
		} \
		break; \
		case 2: for(size_t i=0; i<ins; ++i) \
		{ \
			float frame[2]; \
			PREEMP_DF2_SAMPLE(in, frame, 2) \
			/* Zero-stuffing! Insert zero after making up for energy loss. */ \
			frame[0] *= 2; \
			frame[1] *= 2; \
			LPF_DF2_SAMPLE(times, frame, out, 2) \
			out += 2; \
			frame[1] = frame[0] = 0; \
			LPF_DF2_SAMPLE(times, frame, out, 2) \
			DE_DENORM_FLIP(rd->dede) \
			out += 2; \
			in  += 2; \
		} \
		break; \
		default: for(size_t i=0; i<ins; ++i) \
		{ \
			PREEMP_DF2_SAMPLE(in, rd->frame, rd->channels) \
			/* Zero-stuffing! Insert zero after making up for energy loss. */ \
			for(unsigned int c=0; c<rd->channels; ++c) \
				rd->frame[c] *= 2; \
			LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
			out += rd->channels; \
			for(unsigned int c=0; c<rd->channels; ++c) \
				rd->frame[c] = 0; \
			LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
			DE_DENORM_FLIP(rd->dede) \
			out += rd->channels; \
			in  += rd->channels; \
		} \
	} \
	LPF_DF2_END \
} \
\
static void lowpass##times##_df2_preemp(struct resample_data *rd, float *in, size_t ins) \
{ \
	if(!ins) \
		return; \
	float *out = in; \
	LPF_DF2_BEGIN(times, in, rd->channels,) \
	PREEMP_DF2_BEGIN(in, rd->channels,); \
	switch(rd->channels) \
	{ \
		case 1:  for(size_t i=0; i<ins; ++i) \
		{ \
			float sample; \
			PREEMP_DF2_SAMPLE(in, (&sample), 1) \
			LPF_DF2_SAMPLE(times, (&sample), out, 1) \
			DE_DENORM_FLIP(rd->dede) \
			in++; \
			out++; \
		} \
		break; \
		case 2:  for(size_t i=0; i<ins; ++i) \
		{ \
			float frame[2]; \
			PREEMP_DF2_SAMPLE(in, frame, 2) \
			LPF_DF2_SAMPLE(times, frame, out, 2) \
			DE_DENORM_FLIP(rd->dede) \
			in  += 2; \
			out += 2; \
		} \
		break; \
		default: for(size_t i=0; i<ins; ++i) \
		{ \
			PREEMP_DF2_SAMPLE(in, rd->frame, rd->channels) \
			LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
			DE_DENORM_FLIP(rd->dede) \
			in  += rd->channels; \
			out += rd->channels; \
		} \
	} \
	LPF_DF2_END \
}
#else
#define LOWPASS_DF2_FUNCSX(times) \
\
static void lowpass##times##_df2_preemp_2x(struct resample_data *rd, float *in, size_t ins, float *out) \
{ \
	if(!ins) \
		return; \
	LPF_DF2_BEGIN(times,in,rd->channels,) \
	PREEMP_DF2_BEGIN(in,rd->channels,); \
	for(size_t i=0; i<ins; ++i) \
	{ \
		PREEMP_DF2_SAMPLE(in, rd->frame, rd->channels) \
		/* Zero-stuffing! Insert zero after making up for energy loss. */ \
		for(unsigned int c=0; c<rd->channels; ++c) \
			rd->frame[c] *= 2; \
		LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
		out += rd->channels; \
		for(unsigned int c=0; c<rd->channels; ++c) \
			rd->frame[c] = 0; \
		LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
		DE_DENORM_FLIP(rd->dede) \
		out += rd->channels; \
		in  += rd->channels; \
	} \
	LPF_DF2_END \
} \
\
static void lowpass##times##_df2_preemp(struct resample_data *rd, float *in, size_t ins) \
{ \
	if(!ins) \
		return; \
	float *out = in; \
	LPF_DF2_BEGIN(times, in, rd->channels,) \
	PREEMP_DF2_BEGIN(in, rd->channels,); \
	for(size_t i=0; i<ins; ++i) \
	{ \
		PREEMP_DF2_SAMPLE(in, rd->frame, rd->channels) \
		LPF_DF2_SAMPLE(times, rd->frame, out, rd->channels) \
		DE_DENORM_FLIP(rd->dede) \
		in  += rd->channels; \
		out += rd->channels; \
	} \
	LPF_DF2_END \
}
#endif


// Need that indirection so that times is expanded properly for concatenation.
#define LOWPASS_DF2_FUNCS(times) \
	LOWPASS_DF2_FUNCSX(times)

LOWPASS_DF2_FUNCS(LPF_TIMES)
LOWPASS_DF2_FUNCS(DIRTY_LPF_TIMES)

// Some interpolators based on the deip paper:
// Polynomial Interpolators for High-Quality Resampling of Oversampled Audio
// REVISED VERSION
// by Olli Niemitalo in October 2001
// http://www.student.oulu.fi/~oniemita/DSP/INDEX.HTM
// http://yehar.com/blog/wp-content/uploads/2009/08/deip.pdf

#define OPT4P4O_BEGIN(in) \
	if(!(rd->sflags & inter_flow)) \
	{ \
		for(unsigned int c=0; c<rd->channels; ++c) \
			rd->ch[c].x[0] = rd->ch[c].x[1] = rd->ch[c].x[2] = in[c]; \
		rd->offset = -rd->vinrate; \
		rd->sflags |= inter_flow; \
	}

/* See OPT6P5O_INTERPOL for logic hints. */
#define OPT4P4O_INTERPOL(in, out, outs, channels) \
	{ \
		long sampleoff = rd->offset; \
		if(sampleoff+rd->vinrate < rd->voutrate) \
		{ \
			for(unsigned int c=0; c<channels; ++c) \
			{ \
				float even1 = rd->ch[c].x[2]+rd->ch[c].x[1]; \
				float  odd1 = rd->ch[c].x[2]-rd->ch[c].x[1]; \
				float even2 = in[c]+rd->ch[c].x[0]; \
				float  odd2 = in[c]-rd->ch[c].x[0]; \
				rd->ch[c].c[0] = even1*0.45645918406487612f \
				               + even2*0.04354173901996461f; \
				rd->ch[c].c[1] = odd1*0.47236675362442071f \
				               + odd2*0.17686613581136501f; \
				rd->ch[c].c[2] = even1*-0.253674794204558521f \
				               + even2*0.25371918651882464f; \
				rd->ch[c].c[3] = odd1*-0.37917091811631082f \
				               + odd2*0.11952965967158000f; \
				rd->ch[c].c[4] = even1*0.04252164479749607f \
				               + even2*-0.04289144034653719f; \
			} \
			do	{ \
				sampleoff += rd->vinrate; \
				float z = ((float)sampleoff/rd->voutrate) - (float)0.5; \
				for(unsigned int c=0; c<channels; ++c) \
					out[c] = \
					(	(	( \
								rd->ch[c].c[4] \
								*z + rd->ch[c].c[3] \
							)*z + rd->ch[c].c[2] \
						)*z + rd->ch[c].c[1] \
					)*z + rd->ch[c].c[0]; \
				outs++; \
				out += channels; \
			} while(sampleoff+rd->vinrate < rd->voutrate); \
		} \
		for(unsigned int c=0; c<channels; ++c) \
		{ \
			rd->ch[c].x[0] = rd->ch[c].x[1]; \
			rd->ch[c].x[1] = rd->ch[c].x[2]; \
			rd->ch[c].x[2] = in[c]; \
		} \
		rd->offset = sampleoff - rd->voutrate; \
	}

#define OPT4P4O_END

// Optimal 2x (4-point, 4th-order) (z-form)
static size_t resample_opt4p4o(struct resample_data *rd, float*in
,	size_t ins, float *out)
{
	size_t outs = 0;
	if(!ins)
		return outs;
	OPT4P4O_BEGIN(in)
#ifndef SYN123_NO_CASES
	switch(rd->channels)
	{
		case  1: for(size_t i=0; i<ins; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, 1)
			in++;
		}
		break;
		case  2: for(size_t i=0; i<ins; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, 2)
			in += 2;
		}
		break;
		default:
#endif
		for(size_t i=0; i<ins; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, rd->channels)
			in += rd->channels;
		}
#ifndef SYN123_NO_CASES
	}
#endif
	OPT4P4O_END
	return outs;
}

static size_t resample_opt4p4o_2batch(struct resample_data *rd, float*in, float *out)
{
	size_t outs = 0;
	OPT4P4O_BEGIN(in)
#ifndef SYN123_NO_CASES
	switch(rd->channels)
	{
		case  1: for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, 1)
			in++;
		}
		break;
		case  2: for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, 2)
			in += 2;
		}
		break;
		default:
#endif
		for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT4P4O_INTERPOL(in, out, outs, rd->channels)
			in += rd->channels;
		}
#ifndef SYN123_NO_CASES
	}
#endif
	OPT4P4O_END
	return outs;
}

// This one has low distortion, somewhat more problematic response
// Optimal 2x (6-point, 5th-order) (z-form)

#define OPT6P5O_BEGIN(in) \
	if(!(rd->sflags & inter_flow)) \
	{ \
		for(unsigned int c=0; c<rd->channels; ++c) \
		{ \
			rd->ch[c].x[0] = rd->ch[c].x[1] = rd->ch[c].x[2] \
			=	rd->ch[c].x[3] = rd->ch[c].x[4] = in[c]; \
		} \
		rd->offset = -rd->vinrate; \
		rd->sflags |= inter_flow; \
	}

#define OPT6P5O_INTERPOL(in, out, outs, channels) \
	{ \
		/* Offset is how long ago the last sample occured. */ \
		long sampleoff = rd->offset; \
		/* First check if this interval is interesting at all. */ \
		if(sampleoff < rd->voutrate-rd->vinrate) \
		{ \
			/* Not sure if one big loop over the channels would be better. SIMD?! */ \
			for(unsigned int c=0; c<channels; ++c) \
			{ \
				float even1 = rd->ch[c].x[3]+rd->ch[c].x[2]; \
				float  odd1 = rd->ch[c].x[3]-rd->ch[c].x[2]; \
				float even2 = rd->ch[c].x[4]+rd->ch[c].x[1]; \
				float  odd2 = rd->ch[c].x[4]-rd->ch[c].x[1]; \
				float even3 = in[c]+rd->ch[c].x[0]; \
				float  odd3 = in[c]-rd->ch[c].x[0]; \
				rd->ch[c].c[0] = even1*0.40513396007145713f + even2*0.09251794438424393f \
				               + even3*0.00234806603570670f; \
				rd->ch[c].c[1] = odd1*0.28342806338906690f + odd2*0.21703277024054901f \
				               + odd3*0.01309294748731515f; \
				rd->ch[c].c[2] = even1*-0.191337682540351941f + even2*0.16187844487943592f \
				               + even3*0.02946017143111912f; \
				rd->ch[c].c[3] = odd1*-0.16471626190554542f + odd2*-0.00154547203542499f \
				               + odd3*0.03399271444851909f; \
				rd->ch[c].c[4] = even1*0.03845798729588149f + even2*-0.05712936104242644f \
				               + even3*0.01866750929921070f; \
				rd->ch[c].c[5] = odd1*0.04317950185225609f + odd2*-0.01802814255926417f \
				               + odd3*0.00152170021558204f; \
			} \
			/* Only reaching this code if at least one evaluation is due. */ \
			do { \
				sampleoff += rd->vinrate; \
				/* Evaluating as vector sum may be worthwhile for strong upsampling, */ \
				/* possibly many channels. Otherwise, it's just more muls. */ \
				float z = ((float)sampleoff/rd->voutrate) - (float)0.5; \
				for(unsigned int c=0; c<channels; ++c) \
					out[c] = \
					(	(	(	( \
									rd->ch[c].c[5] \
									*z + rd->ch[c].c[4] \
								)*z + rd->ch[c].c[3] \
							)*z +	rd->ch[c].c[2] \
						)*z +	rd->ch[c].c[1] \
					)*z + rd->ch[c].c[0]; \
				outs++; \
				out += channels; \
			} while(sampleoff < rd->voutrate-rd->vinrate); \
		} \
		for(unsigned int c=0; c<channels; ++c) \
		{ \
			/* Store shifted history. Think about using RING_INDEX here, too, */ \
			rd->ch[c].x[0] = rd->ch[c].x[1]; \
			rd->ch[c].x[1] = rd->ch[c].x[2]; \
			rd->ch[c].x[2] = rd->ch[c].x[3]; \
			rd->ch[c].x[3] = rd->ch[c].x[4]; \
			rd->ch[c].x[4] = in[c]; \
		} \
		rd->offset = sampleoff - rd->voutrate; \
	}

#define OPT6P5O_END

static size_t resample_opt6p5o(struct resample_data *rd, float*in
,	size_t ins, float *out)
{
	size_t outs = 0;
	if(!ins)
		return outs;
	OPT6P5O_BEGIN(in)
#ifndef SYN123_NO_CASES
	switch(rd->channels)
	{
		case  1: for(size_t i=0; i<ins; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, 1)
			in++;
		}
		break;
		case  2: for(size_t i=0; i<ins; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, 2)
			in += 2;
		}
		break;
		default:
#endif
		for(size_t i=0; i<ins; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, rd->channels)
			in += rd->channels;
		}
#ifndef SYN123_NO_CASES
	}
#endif
	OPT6P5O_END
	return outs;
}

// TODO: Remove the 2batch version after verifying that it doesn't improve things.
static size_t resample_opt6p5o_2batch(struct resample_data *rd, float*in
,	float *out)
{
	size_t outs = 0;
	OPT6P5O_BEGIN(in)
#ifndef SYN123_NO_CASES
	switch(rd->channels)
	{
		case 1: for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, 1)
			in++;
		}
		break;
		case 2: for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, 2)
			in += 2;
		}
		break;
		default:
#endif
		for(size_t i=0; i<2*BATCH; ++i)
		{
			OPT6P5O_INTERPOL(in, out, outs, rd->channels)
			in += rd->channels;
		}
#ifndef SYN123_NO_CASES
	}
#endif
	OPT6P5O_END
	return outs;
}

// Full resample function with oversampling.
#define RESAMPLE_FULL_2X(name, lpf_2x, lpf, interpolate_2batch, interpolate) \
static size_t name( struct resample_data *rd \
,	float *in, size_t ins, float *out ) \
{ \
	mxdebug(#name " in %zu", ins); \
	float *iin = in; \
	size_t iins = ins; \
	if(!(rd->sflags & inter_flow) && ins) \
		stage_history_init(rd, 0, iin); \
	size_t outs = 0; \
	size_t nouts = 0; \
	size_t blocks = ins/BATCH; \
	for(size_t bi = 0; bi<blocks; ++bi) \
	{ \
		/* One batch in, two batches out. */ \
		mxdebug(#name " batch %zu lowpass " #lpf_2x, bi); \
		lpf_2x(rd, in, BATCH, rd->upbuf); \
		mxdebug(#name " batch %zu interpol " #interpolate_2batch, bi); \
		nouts = interpolate_2batch(rd, rd->upbuf, out); \
		outs += nouts; \
		out  += nouts*rd->channels; \
		in   += BATCH*rd->channels; \
	} \
	ins -= blocks*BATCH; \
	xdebug(#name " end lowpass " #lpf_2x); \
	lpf_2x(rd, in, ins, rd->upbuf); \
	xdebug(#name " end interpol " #interpolate); \
	nouts = interpolate(rd, rd->upbuf, 2*ins, out); \
	outs += nouts; \
	mxdebug(#name " out %zu", outs); \
	stage_history(rd, 0, iin, iins); \
	return outs; \
}

// Full resample method without oversampling.
#define RESAMPLE_FULL_1X(name, lpf, interpolate) \
static size_t name( struct resample_data *rd \
,	float *in, size_t ins, float *out ) \
{ \
	mxdebug(#name " in %zu", ins); \
	float *iin = in; \
	size_t iins = ins; \
	if(!(rd->sflags & inter_flow) && ins) \
		stage_history_init(rd, 0, iin); \
	size_t outs = 0; \
	size_t nouts = 0; \
	size_t blocks = ins/BATCH; \
	for(size_t bi = 0; bi<blocks; ++bi) \
	{ \
		mxdebug(#name " batch %zu lowpass " #lpf, bi); \
		memcpy(rd->prebuf, in, BATCH*sizeof(*in)*rd->channels); \
		lpf(rd, rd->prebuf, BATCH); \
		mxdebug(#name " batch %zu interpol " #interpolate, bi); \
		nouts = interpolate(rd, rd->prebuf, BATCH, out); \
		outs += nouts; \
		out  += nouts*rd->channels; \
		in   += BATCH*rd->channels; \
	} \
	ins -= blocks*BATCH; \
	xdebug(#name " end lowpass " #lpf); \
	memcpy(rd->prebuf, in, ins*sizeof(*in)*rd->channels); \
	lpf(rd, rd->prebuf, ins); \
	xdebug(#name " end interpol " #interpolate); \
	nouts = interpolate(rd, rd->prebuf, ins, out); \
	outs += nouts; \
	mxdebug(#name " out %zu", outs); \
	stage_history(rd, 0, iin, iins); \
	return outs; \
}

// Full resample method with decimation.
#define RESAMPLE_FULL_0X(name, lpf, interpolate) \
static size_t name( struct resample_data *rd \
,	float *in, size_t ins, float *out ) \
{ \
	mxdebug(#name " in %zu", ins); \
	float *iin = in; \
	size_t iins = ins; \
	if(!(rd->sflags & inter_flow) && ins) \
		stage_history_init(rd, 0, iin); \
	size_t outs = 0; \
	size_t nouts = 0; \
	size_t blocks = ins/BATCH; \
	for(size_t bi = 0; bi<blocks; ++bi) \
	{ \
		mxdebug( #name " batch %zu decim %d and lowpass " #lpf \
		,	bi, rd->decim_stages ); \
		int fill = BATCH; \
		memcpy(rd->prebuf, in, fill*sizeof(*in)*rd->channels); \
		for(unsigned int ds = 0; ds<rd->decim_stages; ++ds) \
			fill = decimate(rd, ds, rd->prebuf, fill); \
		lpf(rd, rd->prebuf, fill); \
		mxdebug(#name " batch %zu interpol " #interpolate, bi); \
		nouts = interpolate(rd, rd->prebuf, fill, out); \
		outs += nouts; \
		out  += nouts*rd->channels; \
		in   += BATCH*rd->channels; \
	} \
	ins -= blocks*BATCH; \
	int fill = ins; \
	mxdebug( #name " end decim %d and lowpass " #lpf \
	, rd->decim_stages ); \
	memcpy(rd->prebuf, in, fill*sizeof(*in)*rd->channels); \
	for(unsigned int ds = 0; ds<rd->decim_stages; ++ds) \
		fill = decimate(rd, ds, rd->prebuf, fill); \
	lpf(rd, rd->prebuf, fill); \
	xdebug(#name " end interpol " #interpolate); \
	nouts = interpolate(rd, rd->prebuf, fill, out); \
	outs += nouts; \
	mxdebug(#name " out %zu", outs); \
	stage_history(rd, 0, iin, iins); \
	return outs; \
}

// Define the set of resamplers for a given set of lowpasses and interpolators.
#define RESAMPLE_FULL( name_2x, name_1x, name_0x \
, lpf_2x, lpf, interpolate_2batch, interpolate ) \
RESAMPLE_FULL_2X(name_2x, lpf_2x, lpf, interpolate_2batch, interpolate) \
RESAMPLE_FULL_1X(name_1x, lpf, interpolate) \
RESAMPLE_FULL_0X(name_0x, lpf, interpolate)

// Finally define our dirty and fine resamplers.
RESAMPLE_FULL( resample_2x_dirty, resample_1x_dirty, resample_0x_dirty
,	DIRTY_LOWPASS_PREEMP_2X, DIRTY_LOWPASS_PREEMP
,	resample_opt4p4o_2batch, resample_opt4p4o )
RESAMPLE_FULL( resample_2x_fine,  resample_1x_fine,  resample_0x_fine
,	LOWPASS_PREEMP_2X, LOWPASS_PREEMP
,	resample_opt6p5o_2batch, resample_opt6p5o )

// Here I go and implement a bit of big number functionality.
// The task is just to truly be able to compute a*b/c with unsigned
// integers for cases where the result does not overflow but the intermediate
// product may not fit into the type.
// I do not want to rely on a 128 bit type being availbe, so I take
// uint64_t and run with it.

// Multiply two 64 bit unsigned integers and return the result as two
// halves of 128 bit.
static void mul64(uint64_t a, uint64_t b, uint64_t * m1, uint64_t * m0)
{
	uint64_t lowmask = ((uint64_t)1 << 32) - 1;
	uint64_t a1 = a >> 32;
	uint64_t a0 = a & lowmask;
	uint64_t b1 = b >> 32;
	uint64_t b0 = b & lowmask;
	uint64_t prod1 = a1 * b1;
	uint64_t prod0 = a0 * b0;
	uint64_t a1b0 = a1 * b0;
	uint64_t a0b1 = a0 * b1;
	prod1 += a0b1 >> 32;
	prod1 += a1b0 >> 32;
	uint64_t prod0t = prod0 + ((a0b1 & lowmask) << 32);
	if(prod0t < prod0)
		prod1 += 1;
	prod0 = prod0t + ((a1b0 & lowmask) << 32);
	if(prod0 < prod0t)
		prod1 += 1;
	*m1 = prod1;
	*m0 = prod0;
}

// Multiply two unsigned integers numbers and divide by a third one,
// returning the result in the same integer width, if no overflow
// occurs. Also catches division by zero. On error, zero is returned
// and the error code set to a non-zero value (1==overflow, 2==divbyzero).
// The division result is computed on two halves of 128 bit internally,
// and discarded if the higher half contains set bits.
// Also, if m != NULL, the remainder is stored there.
// The actually interesting function for the interpolation:
// e = (a*b+off)/d
// This is bounded by zero: If (a*b+off) < 0, this returns just
// zero, also zero remainder.
static uint64_t muloffdiv64( uint64_t a, uint64_t b, int64_t off, uint64_t c
,	int *err, uint64_t *m )
{
	uint64_t prod1, prod0;
	uint64_t div1, div0, div0old;
	if(c < 1)
	{
		if (err)
			*err = 2;
		return 0;
	}
	mul64(a, b, &prod1, &prod0);
	if(off)
	{
		uint64_t prod0old = prod0;
		prod0 += off;
		// Offset can be positive or negative, small or large.
		// When adding to prod0, these cases can happen:
		// offset > 0, result > prod0: All fine.
		// offset > 0, result < prod0: Overflow.
		// offset < 0, result < prod0: All fine.
		// offset < 0, result > prod0: Underflow.
		if(off > 0 && prod0 < prod0old)
		{
			if(prod1 == UINT64_MAX)
			{
				if(err)
					*err = 1;
				return 0;
			}
			++prod1;
		}
		if(off < 0 && prod0 > prod0old)
		{
			// Pin to zero on total underflow.
			if(prod1 == 0)
				prod0 = 0;
			else
				--prod1;
		}
	}
	if(c == 1)
	{
		div1 = prod1;
		div0 = prod0;
		if(m)
			*m = 0;
	} else
	{
		div1 = 0;
		div0 = 0;
		uint64_t ctimes = ((uint64_t) - 1) / c;
		uint64_t cblock = ctimes * c;
		while(prod1)
		{
			uint64_t cmult1, cmult0;
			mul64(ctimes, prod1, &cmult1, &cmult0);
			div1 += cmult1;
			div0old = div0;
			div0 += cmult0;
			if(div0 < div0old)
				div1++;
			mul64(cblock, prod1, &cmult1, &cmult0);
			prod1 -= cmult1;
			if(prod0 < cmult0)
				prod1--;
			prod0 -= cmult0;
		}
		div0old = div0;
		div0 += prod0 / c;
		if(m)
			*m = prod0 % c;
		if(div0 < div0old)
			div1++;
	}
	if(div1)
	{
		if(err)
			*err = 1;
		return 0;
	}
	if(err)
		*err = 0;
	return div0;
}

// Ensure that the sample rates are not above half of the
// possible range of the used data type, including a safeguard
// that 64 bits will always be enough to work with the values.
// This limits you to 63 bits precision in your sampling rate
// ratio for all times, or a maximum speedup/slowdown by 4.6e18.
// This is still a rather big number.
#if LONG_MAX > INT64_MAX
#define RATE_LIMIT INT64_MAX/2
#else
#define RATE_LIMIT LONG_MAX/2
#endif

long attribute_align_arg
syn123_resample_maxrate(void)
{
	return RATE_LIMIT;
}

// Settle oversampling and decimation stages for the given
// input/output rate ratio.
static int rate_setup( long inrate, long outrate
,	int *oversample, unsigned int *decim_stages )
{
	// Want enough headroom so that the sum of two rates can be represented.
	// And of course we don't do reverse or frozen time.
	if( inrate < 1 || inrate  > RATE_LIMIT ||
	   outrate < 1 || outrate > RATE_LIMIT  )
		return -1;
	*oversample = outrate*2 > inrate ? 1 : 0;
	*decim_stages = 0;
	if(outrate <= LONG_MAX/4) while(outrate*4 < inrate)
	{
		(*decim_stages)++;
		outrate *= 2;
	}
	return 0;
}

// Revisit the cases:
// 2x: to keep the implementation free, one must assume to be able to hold
//     2*ins in the data type, then 2*ins/2*inrate*outrate+1 = ins*outrate/inrate+1
// 1x: no doubling, just ins/inrate*outrate+1
//     but since the rate to interpolate to is <= 1/2 input, outsamples
//     always will be less, so SIZE_MAX
// 0x: output < 1/4 input. Rounding error of 2 does not matter. Things get smaller,
//     so SIZE_MAX.

size_t attribute_align_arg
syn123_resample_maxincount(long inrate, long outrate)
{
	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return 0;
	if(oversample)
	{
		uint64_t outlimit;
		// I'm crazy. I am planning for size_t being more than 64 bits.
		if(SIZE_MAX-1 > UINT64_MAX)
			outlimit = UINT64_MAX;
		else
			outlimit = SIZE_MAX-1;
		// SIZE_MAX = ins*outrate/inrate+1
		// SIZE_MAX-1 = ins*outrate/inrate
		// (SIZE_MAX-1)*inrate = ins*outrate
		// ins = (SIZE_MAX-1)*inrate/outrate
		// Integer math always rounding down, this must be below the limit.
		int err;
		uint64_t maxin = muloffdiv64(outlimit, inrate, 0, outrate, &err, NULL);
		// The only possible error is genuine overflow. Meaning: We can
		// give as much input as can be represented in the variable.
		if(err)
			maxin = UINT64_MAX;
		return maxin > SIZE_MAX ? SIZE_MAX : (size_t)maxin;
	}
	else
		return SIZE_MAX;
}

// The history is returned as size_t, which usually provide plenty of range.
// The idea is that the history needs to fit into memory, although that is not
// really the case. It is just impractical to consider the full history of
// extreme resampling ratios.
size_t attribute_align_arg
syn123_resample_history(long inrate, long outrate, int dirty)
{
	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return 0;
	if(oversample && decim_stages)
		return 0;
	// Either 4p4o or 6p5o interpolation at the end. We only need the points
	// before the current sample in the history, so one less.
	size_t history = (dirty ? DIRTY_POINTS : FINE_POINTS) - 1;
	// For the oldest of these, we need a history of input before it to
	// fill the low pass histories. The number of previous low pass points
	// simply comes on top. The number of low pass applications does not matter.
	history += LPF_HISTORY(LPF_ORDER);

	if(oversample)
	{
		// Need only half the input, but no less.
		history = (history+1)/2;
	}
	// Here, the numbers can potentially become very large.
	// Each decimation step doubles the needed input samples and
	// adds the state of the decimation filter.
	while(decim_stages--)
	{
		// Reverse decimation ...
		if(history > SIZE_MAX/2+1)
			return SIZE_MAX;
		history = (history-1) + history;
		if(history > SIZE_MAX-LPF_HISTORY(LPF_4_ORDER))
			return SIZE_MAX;
		history += LPF_HISTORY(LPF_4_ORDER);
	}
	return history;
}

#if 0
// This should be the essence of what the interpolation does regarding
// input and output sample count.
int64_t simulate_interpolate(long vinrate, long voutrate, int64_t ins)
{
	int64_t outs = 0;
	long offset = -vinrate;
	for(int64_t n = 0; n<ins; ++n)
	{
		while(offset+vinrate < voutrate)
		{
			++outs;
			offset += vinrate;
		}
		offset -= voutrate;
	}
	return outs;
}
#endif

// The exact output sample count given total input size.
// It assumes zero history.
// This returns a negative code on error.
// The combination of interpolation and stages of decimation calls
// for more than simple integer division. The overall rounding error
// is limited at 2 samples, btw.
int64_t attribute_align_arg
syn123_resample_total64(long inrate, long outrate, int64_t ins)
{
	if(ins < 0)
		return -1;
	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return SYN123_BAD_FMT;
	if(oversample && decim_stages)
		return SYN123_WEIRD;

	uint64_t vins = ins;
	uint64_t vinrate = inrate;
	uint64_t voutrate = outrate;
	if(decim_stages)
	{
		// The interpolation uses the virtual output rate to get the same
		// ratio as with decimated input rate.
		voutrate <<= decim_stages;
		// The first sample of a block gets used, the following are dropped.
		// So I get one sample out of the first input sample, regardless of
		// decimation stage count. But the next sample comes only after
		// 2^decim_stages further input samples.
		// The formula for one stage: y = (x+1)/2
		// For n stages ... my guess is wrong. Heck, n is small, less than
		// 64 (see RATE_LIMIT). Let's just mimick the decimations in a loop.
		// Since vins < UINT64_MAX/2, adding 1 is no issue.
		for(unsigned int i=0; i<decim_stages; ++i)
			vins = (vins+1)/2;
	}
	if(oversample)
	{
		vins    *= 2;
		vinrate *= 2;
	}
	int err;
	// Interpolation model:
	//   outs = (vins*voutrate - offset - 1)/vinrate
	// with offset = -vinrate at the beginning. This is the rounding, no
	// need to deal with the remainder.
	uint64_t tot = muloffdiv64(vins, voutrate, vinrate-1, vinrate, &err, NULL);
	// Any error is treated as overflow (div by zero -> infinity).
	if(err)
		return SYN123_OVERFLOW;
	return (tot <= INT64_MAX) ? (int64_t)tot : SYN123_OVERFLOW;
}

// The inverse function: How many input samples are needed to get at least
// the desired amount of output?
int64_t attribute_align_arg
syn123_resample_intotal64(long inrate, long outrate, int64_t outs)
{
	if(outs < 1)
		return outs == 0 ? 0 : -1;
	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return SYN123_BAD_FMT;
	if(oversample && decim_stages)
		return SYN123_WEIRD;
	uint64_t voutrate = outrate;
	voutrate <<= decim_stages;
	uint64_t vinrate  = inrate;
	if(oversample)
		vinrate *= 2;
	// This works for outs > 0:
	// ins = (outs*inrate+offset)/outrate+1
	// First offset is -inrate.
	// You may want to work it out for yourself. Or trust me;-)
	int err;
	uint64_t vtot = muloffdiv64(outs, vinrate, -(int64_t)vinrate, voutrate, &err, NULL);
	if(err)
		return SYN123_OVERFLOW;
	if(vtot == UINT64_MAX)
		return SYN123_OVERFLOW;
	++vtot; // Must be at least 1 now!
	uint64_t tot = vtot;
	// Un-do oversampling, taking care not to hit overflow.
	if(oversample)
	{
		tot /= 2;
		if(tot*2 < vtot)
			++tot;
	}
	// Now tot is the minimum needed input sample count before interpolation.
	// Decimation still needs to be accounted for.
	// With decimation, I need the smallest input that fulfills out=(2*in+1)/2.
	for(unsigned int i=0; i<decim_stages; ++i)
	{
		if(!tot)
			return 0;
		if(tot > UINT64_MAX/2+1)
			return SYN123_OVERFLOW;
		tot = (tot - 1) + tot;
	}
	return (tot <= INT64_MAX) ? (int64_t)tot : SYN123_OVERFLOW;
}

#ifndef PORTABLE_API
// Again the dreadful business of dealing with shape-shifting off_t API.
// The real implementation is the function with suffix 64, using int64_t.
// Depending on your OS, there is a native off_t with 32 or 64 bits, and
// possibly a 64 bit off64_t. The macros here define the appropriate wrappers
// for the _32 and _64 suffixes promised in the API header, depending on
// _FILE_OFFSET_BITS value.
// A special case is a system that does ignore _FILE_OFFSET BITS but has
// 64 bit offsets: That should still get the plain alias and one with _64,
// making the header agnostic to that fact, not depending on any build properties

#define resample_total_wrap(type, limit, name, name64) \
type attribute_align_arg name(long inrate, long outrate, type io) \
{ \
	int64_t tot = name64(inrate, outrate, io); \
	return (tot <= limit) ? (type)tot : SYN123_OVERFLOW; \
}
#define resample_total_alias(type, name, name64) \
type attribute_align_arg name(long inrate, long outrate, type io) \
{ \
	return name64(inrate, outrate, io); \
}

#if  SIZEOF_OFF_T == 8
#ifndef FORCED_OFF_64
resample_total_alias(off_t, syn123_resample_total, syn123_resample_total64)
resample_total_alias(off_t, syn123_resample_intotal, syn123_resample_intotal64)
#endif
resample_total_alias(off_t, syn123_resample_total_64, syn123_resample_total64)
resample_total_alias(off_t, syn123_resample_intotal_64, syn123_resample_intotal64)
#elif SIZEOF_OFF_T == 4
resample_total_wrap(off_t, INT32_MAX, syn123_resample_total, syn123_resample_total64)
resample_total_wrap(off_t, INT32_MAX, syn123_resample_intotal, syn123_resample_intotal64)
resample_total_wrap(off_t, INT32_MAX, syn123_resample_total_32, syn123_resample_total64)
resample_total_wrap(off_t, INT32_MAX, syn123_resample_intotal_32, syn123_resample_intotal64)
#ifdef LFS_LARGEFILE_64
resample_total_alias(off64_t, syn123_resample_total_64, syn123_resample_total64)
resample_total_alias(off64_t, syn123_resample_intotal_64, syn123_resample_intotal64)
#endif
#else
#error "Unexpected off_t size."
#endif

#endif

// As any sensible return value is at least 1, this uses the unsigned
// type and 0 for error/pathological input.
// This function could be simplified to
//     uintmax_t count = prod/inrate + (prod%inrate ? 1 : 0) + 1;
// to accomodate for possible rounding during decimation and interpolation,
// but it would be a teensy bit less accurate. The computation time
// should not matter much. The simple formula can be used as a test
// to check if the elaborate function comes to a value that only
// sometimes is less by 1, but not more.
size_t attribute_align_arg
syn123_resample_count(long inrate, long outrate, size_t ins)
{
	// The largest output you get from the beginning. So this can just treat ins
	// as offset from there.
	if( ins > syn123_resample_maxincount(inrate, outrate)
#if SIZE_MAX > INT64_MAX
		|| ins > INT64_MAX
#endif
	)
		return 0;
	int64_t tot = syn123_resample_total64(inrate, outrate, (int64_t)ins);
	return (tot >= 0 && tot <= SIZE_MAX) ? (size_t)tot : 0;
}

size_t attribute_align_arg
syn123_resample_incount(long inrate, long outrate, size_t outs)
{
	// I had logic here about abusing syn123_resample_intotal(), but that is wrong.
	// Do not assume you know what interval to pick to get the maximum value.
	// Work the algorithm for the theoretical maximum at each point, even if that
	// may not be hit with your particular rate ratio and playback time.
#if SIZE_MAX > INT64_MAX
	if(outs > INT64_MAX-1)
		return 0;
#endif
	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return 0;
	if(oversample && decim_stages)
		return 0;
	uint64_t voutrate = outrate;
	voutrate <<= decim_stages;
	uint64_t vinrate  = inrate;
	if(oversample)
		vinrate *= 2;
	// This works for outs > 0:
	// ins = (outs*inrate+offset)/outrate+1
	// First offset is -inrate, maximum offset is -1. I think.
	// You may want to work it out for yourself. Or trust me;-)
	int err;
	uint64_t vtot = muloffdiv64(outs, vinrate, -1, voutrate, &err, NULL);
	if(err)
		return SYN123_OVERFLOW;
	if(vtot == UINT64_MAX)
		return SYN123_OVERFLOW;
	++vtot; // Must be at least 1 now!
	uint64_t tot = vtot;
	// Un-do oversampling, taking care not to hit overflow.
	if(oversample)
	{
		tot /= 2;
		if(tot*2 < vtot)
			++tot;
	}
	// Now tot is the minimum needed input sample count before interpolation.
	// Decimation still needs to be accounted for. Worst case: We got no
	// sample buffered (dropped) in any stage: So just multiply.
	if(tot > UINT64_MAX>>decim_stages)
		return SYN123_OVERFLOW;
	tot<<=decim_stages;
	return (tot <= SIZE_MAX) ? (size_t)tot : 0;
}

// A bit of convenience. Just rely on the other functions.
size_t attribute_align_arg
syn123_resample_fillcount(long input_rate, long output_rate, size_t outs)
{
	size_t block = syn123_resample_incount(input_rate, output_rate, outs);
	// Reduce that to ensure that we never get more than a buffer's fill.
	while( block &&
		syn123_resample_count(input_rate, output_rate, block) > outs )
		--block;
	return block;
}

// Store given error value to storage, if non-NULL, return
// given return value for pass-through usage.
static size_t size_err(int *err, int value, size_t ret)
{
	if(err)
		*err = value;
	return ret;
}

// The exact predictor: How many output samples will I get _now_
// after feeding the indicated amount? This is concerned with
// Buffer sizes, so let's drop the 32/64 bit distinction.
// Since overflow can occur, we need a sign bit to signal errors.
size_t attribute_align_arg
syn123_resample_out(syn123_handle *sh, size_t ins, int *err)
{
	size_err(err, SYN123_OK, 0);
	if(!sh || !sh->rd)
		return size_err(err, SYN123_BAD_HANDLE, 0);
	if(ins < 1)
		return 0;
	struct resample_data *rd = sh->rd;
	// Need to account for the current resampler state. That is:
	// 1. decimation stages having swallowed a sample or not,
	// 2. current interpolator offset.
	// The first sample of a block gets used, the following are dropped.
	// So I get one sample out of the first input sample, regardless of
	// decimation stage count. But the next sample comes only after
	// 2^decim_stages further input samples.
	// The formula for one stage: y = (x+1)/2
	// For n stages ... my guess is wrong. Heck, n is small, less than
	// 64 (see RATE_LIMIT). Let's just mimick the decimations in a loop.
	// Since vins < UINT64_MAX/2, adding 1 is no issue.
	for(unsigned int i=0; i<rd->decim_stages; ++i)
	{
		// If the stage is fresh or has explictly a past sample in memorian,
		// we can add one virtual input sample. Divide first to avoid overflow.
		size_t nins = ins / 2;
		// When there is one sample left over, a second one from storage
		// adds to the decimated count.
		if( nins*2 < ins && (!(rd->decim[i].sflags & lowpass_flow)
		||	(rd->decim[i].sflags & decimate_store)) )
			++nins;
		ins = nins;
	}
#if SIZE_MAX > UINT64_MAX
	if(ins > UINT64_MAX) // Really?
		return size_err(err, SYN123_OVERFLOW, 0);
#endif
	uint64_t vins = ins;
	if(rd->sflags & oversample_2x)
	{
		if(vins > UINT64_MAX/2)
			return size_err(err, SYN123_OVERFLOW, 0);
		vins *= 2;
	}
	int myerr = SYN123_OK;
	int64_t offset = rd->sflags & inter_flow ? rd->offset : -rd->vinrate;
	// Interpolation model:
	//   outs = (vins*voutrate - offset - 1)/vinrate
	uint64_t tot = muloffdiv64( vins, (uint64_t)rd->voutrate
	,	(int64_t)(-offset-1), (uint64_t)rd->vinrate, &myerr, NULL );
	// Any error is treated as overflow (div by zero -> infinity).
	if(myerr || tot > SIZE_MAX)
		return size_err(err, SYN123_OVERFLOW, 0);
	return (size_t)tot;
}

syn123_ssize_t attribute_align_arg
syn123_resample_expect(syn123_handle *sh, size_t ins)
{
	int err = 0;
	size_t tot = syn123_resample_out(sh, ins, &err);
	if(tot >= SSIZE_MAX)
		err = SYN123_OVERFLOW;
	return (syn123_ssize_t)(err ? err : tot);
}

// How many input samples are minimally needed to get at least the
// minimally desired output right now?
size_t attribute_align_arg
syn123_resample_in(syn123_handle *sh, size_t outs, int *err)
{
	size_err(err, SYN123_OK, 0);
	if(!sh || !sh->rd)
		return size_err(err, SYN123_BAD_HANDLE, 0);
	if(outs < 1)
		return 0;
	struct resample_data *rd = sh->rd;
	// This works for outs > 0:
	// ins = (outs*inrate+offset)/outrate+1
	int myerr;
	int64_t offset = rd->sflags & inter_flow ? rd->offset : -rd->vinrate;
	uint64_t vtot = muloffdiv64(outs, sh->rd->vinrate, offset, rd->voutrate
	,	&myerr, NULL);
	if(myerr || vtot == UINT64_MAX)
		return size_err(err, SYN123_OVERFLOW, 0);
	++vtot;
	uint64_t tot = vtot;
	// Un-do oversampling, taking care not to hit overflow.
	if(rd->sflags & oversample_2x)
	{
		tot /= 2;
		if(tot*2 < vtot)
			++tot;
	}
	// Still: tot > 0
	// Got needed output of deciation, for each stage
	// a) need twice that amount if there is no sample buffered
	// b) need one less than twice the amount if there is a sample buffered
	for(unsigned int j=0; j<rd->decim_stages; ++j)
	{
		unsigned int i = rd->decim_stages - 1 - j;
		if(tot > UINT64_MAX/2+1)
			return size_err(err, SYN123_OVERFLOW, 0);
		tot = (tot - 1) + tot;
		if( (rd->decim[i].sflags & lowpass_flow)
		&&	!(rd->decim[i].sflags & decimate_store) )
		{
			if(tot == UINT64_MAX)
				return size_err(err, SYN123_OVERFLOW, 0);
			++tot;
		}
	}
	return (size_t)(tot <= SIZE_MAX ? tot : size_err(err, SYN123_OVERFLOW, 0));
}

syn123_ssize_t attribute_align_arg
syn123_resample_inexpect(syn123_handle *sh, size_t outs)
{
	int err = 0;
	size_t tot = syn123_resample_in(sh, outs, &err);
	if(tot >= SSIZE_MAX)
		err = SYN123_OVERFLOW;
	return (syn123_ssize_t)(err ? err : tot);
}

void resample_free(struct resample_data *rd)
{
	if(!rd)
		return;
	if(rd->stage_history)
		free(rd->stage_history);
	if(rd->decim_hist)
		free(rd->decim_hist);
	if(rd->decim)
		free(rd->decim);
	if(rd->upbuf)
		free(rd->upbuf);
	if(rd->prebuf)
		free(rd->prebuf);
	if(rd->ch)
		free(rd->ch);
	if(rd->frame)
		free(rd->frame);
	free(rd);
}

// Want to support smooth rate changes.
// If there is a handle present, try to keep as much as possible.
// If you change the dirty flag, things get refreshed totally.
int attribute_align_arg
syn123_setup_resample( syn123_handle *sh, long inrate, long outrate
,	int channels, int dirty, int smooth )
{
	int err = 0;
	struct resample_data *rd = NULL;
	int fresh = 0;

	if(inrate == 0 && outrate == 0 && channels == 0)
	{
		err = SYN123_BAD_FMT;
		goto setup_resample_cleanup;
	}

	if(channels < 1)
	{
		err = SYN123_BAD_FMT;
		goto setup_resample_cleanup;
	}

	// If dirty setyp changed, start anew.
	// TODO: implement proper smooth rate change.
	if( sh->rd && ( (sh->rd->channels != channels)
	||	(!(sh->rd->sflags & dirty_method) != !dirty)
	||	(!(sh->rd->sflags & smooth_change) != !smooth) ) )
	{
		resample_free(sh->rd);
		sh->rd = NULL;
	}

	int oversample;
	unsigned int decim_stages;
	if(rate_setup(inrate, outrate, &oversample, &decim_stages))
		return SYN123_BAD_FMT;
	if(oversample && decim_stages)
		return SYN123_WEIRD;
	long voutrate = outrate<<decim_stages;
	long vinrate  = oversample ? inrate*2 : inrate;
	if(!sh->rd)
	{
		fresh = 1;
		sh->rd = malloc(sizeof(*(sh->rd)));
		if(!sh->rd)
			return SYN123_DOOM;
		memset(sh->rd, 0, sizeof(*(sh->rd)));
		rd = sh->rd;
		rd->inrate = rd->vinrate = rd->voutrate = rd->outrate = -1;
		rd->resample_func = NULL;
		rd->lowpass_func    = dirty ? DIRTY_LOWPASS_PREEMP    : LOWPASS_PREEMP;
		rd->lowpass_2x_func = dirty ? DIRTY_LOWPASS_PREEMP_2X : LOWPASS_PREEMP_2X;
		rd->decim = NULL;
		rd->decim_hist = NULL;
		rd->channels = channels;
		rd->stage_history = NULL;
		DE_DENORM_INIT(rd->dede, +1)
		rd->frame = NULL;
#ifndef SYN123_NO_CASES
		if(channels > 2)
#endif
		{
			rd->frame = malloc(sizeof(float)*channels);
			if(!rd->frame)
			{
				free(rd);
				return SYN123_DOOM;
			}
		}
		rd->ch = malloc(sizeof(struct channel_history)*channels);
		rd->prebuf = malloc(sizeof(float)*channels*BATCH);
		rd->upbuf  = malloc(sizeof(float)*channels*2*BATCH);
		if(!rd->ch || !rd->prebuf || !rd->upbuf)
		{
			if(rd->upbuf)
				free(rd->upbuf);
			if(rd->prebuf)
				free(rd->prebuf);
			if(rd->ch)
				free(rd->ch);
			if(rd->stage_history)
				free(rd->stage_history);
			if(rd->frame)
				free(rd->frame);
			free(rd);
			return SYN123_DOOM;
		}
		memset(rd->ch, 0, sizeof(struct channel_history)*channels);
	}
	else
		rd = sh->rd;

	mdebug("init in %ld out %ld", inrate, outrate);
	// To support on-the-fly rate changes, superfluous decimator stages are
	// forgotten, new ones added as needed.
	// If smoothing is desired and the stream already flowing, new stages will
	// get started on the previously utmost stage's history.
	if(fresh || decim_stages != rd->decim_stages)
	{
		if(smooth)
		{
			float *sth = INT123_safe_realloc( rd->stage_history
			,	sizeof(float)*STAGE_HISTORY*channels*(decim_stages+1) );
			if(!sth)
			{
				err = SYN123_DOOM;
				goto setup_resample_cleanup;
			}
			rd->stage_history = sth;
		}
		if(decim_stages)
		{
			struct decimator_state *nd = INT123_safe_realloc( rd->decim
			,	sizeof(*rd->decim)*decim_stages );
			struct lpf4_hist *ndh = INT123_safe_realloc( rd->decim_hist
			,	sizeof(*rd->decim_hist)*decim_stages*channels );
			if(nd)
				rd->decim = nd;
			if(ndh)
				rd->decim_hist = ndh;
			if(!nd || !ndh)
			{
				perror("cannot allocate decimator state");
				err = SYN123_DOOM;
				goto setup_resample_cleanup;
			}
		} else
		{
			free(rd->decim); rd->decim = NULL;
			free(rd->decim_hist); rd->decim_hist = NULL;
		}
		// Link up the common memory blocks after each realloc.
		for(unsigned int dc=0; dc<decim_stages; ++dc)
		{
			rd->decim[dc].ch = rd->decim_hist+dc*channels;
			rd->decim[dc].out_hist = rd->stage_history
			?	rd->stage_history+(dc+1)*STAGE_HISTORY*channels
			:	NULL;
		}
		// Smoothness does matter if the interpolator produced something
		// previously, so inter_flow is a good flag to test.
		int init_stages = rd->stage_history && rd->sflags & inter_flow;
		// The situation: Got stage rd->decim_stages (possibly zero).
		// with proper output history. Need to start up the additional
		// stages with the output of the preceeding stages. I'll pretend
		// each stage history being complete, even if I know that I am
		// halving the count in each step.
		for(unsigned int dc=rd->decim_stages; dc<decim_stages; ++dc)
		{
			rd->decim[dc].sflags = 0;
			if(init_stages)
			{
				// Take previous stage output dc, process through
				// the new stage and have it make up its output history.
				// As decimate() overwrites its input, work on a copy.
				memcpy( rd->prebuf
				,	rd->stage_history+dc*STAGE_HISTORY*rd->channels
				,	STAGE_HISTORY*rd->channels*sizeof(float) );
				decimate(rd, dc, rd->prebuf, STAGE_HISTORY);
			}
		}
		rd->decim_stages = decim_stages;
	}

	if(rd->sflags & inter_flow)
	{
		// If the virtual input rate changes (esp. increases), the interpolation
		// offset needs adjustment to stay at/below predicted sample counts.
		// Could always re-initialize the interpolation, but want to keep things
		// smooth.
		long sign = rd->offset < 0 ? -1 : +1;
		// Zero on overflow, suits me just fine.
		uint64_t noff = muloffdiv64( (uint64_t)(sign*rd->offset)
		,	(uint64_t)vinrate, 0, (uint64_t)rd->vinrate, NULL, NULL );
		// Magnitude must not be too large. Too small is OK.
		if(noff > LONG_MAX)
			noff = LONG_MAX;
		rd->offset = sign*(long)noff;
		// Total paranoia. This is the crucial condition.
		if(rd->offset < -vinrate)
			rd->offset = -vinrate;
	}
	rd->inrate = inrate;
	rd->outrate = outrate;
	rd->vinrate = vinrate;
	rd->voutrate = voutrate;
	if(oversample)
		rd->sflags |= oversample_2x;
	else
		rd->sflags &= ~oversample_2x;

	// TODO: channels!
	if(rd->sflags & oversample_2x)
		rd->resample_func = dirty ? resample_2x_dirty : resample_2x_fine;
	else if(rd->decim_stages)
		rd->resample_func = dirty ? resample_0x_dirty : resample_0x_fine;
	else
		rd->resample_func = dirty ? resample_1x_dirty : resample_1x_fine;

	if(dirty)
		rd->sflags |= dirty_method;
	if(smooth)
		rd->sflags |= smooth_change;

	mdebug( "%u times decimation by 2"
		", virtual output rate %ld, %ldx oversampling (%i)"
	,	rd->decim_stages, rd->voutrate, rd->vinrate / rd->inrate
	,	rd->sflags && oversample_2x ? 1 : 0 );

	// Round result to single precision, but compute with double to be able
	// to correctly represent 32 bit longs first.
	rd->lpf_cutoff = (float)((double)(rd->inrate < rd->outrate ? rd->inrate : rd->voutrate)/rd->vinrate);
	mdebug( "lowpass cutoff: %.8f of virtual %.0f Hz = %.0f Hz"
	,	rd->lpf_cutoff, 0.5*rd->vinrate, rd->lpf_cutoff*0.5*rd->vinrate );
	mdebug("rates final: %ld|%ld -> %ld|%ld"
	,	rd->inrate, rd->vinrate, rd->voutrate, rd->outrate );
	preemp_init(rd);
	if(rd->sflags & lowpass_flow)
	{
		// First cheap stage of rate change smoothing:
		// Attempt to dampen spikes from the filter history magnitudes
		// not matching changed filter setup by simple rescaling.
		// This catches the massive spikes on adding decimation stages.
		float scale = lpf_history_scale(rd);
		lpf_init(rd);
		mdebug( "smooth old scale: %g, new scale: %g"
		,	scale, lpf_history_scale(rd) );
		scale = scale > 1e-10f ? lpf_history_scale(rd) / scale : 1.f;
		for(unsigned int c=0; c<rd->channels; ++c)
			for(unsigned int j=0; j<LPF_MAX_TIMES; ++j)
				for(unsigned int i=0; i<LPF_ORDER; ++i)
					rd->ch[c].lpf_w[j][i] *= scale;
		// Full smoothness with present stage history: Let the lowpass roll.
		// Preemp needs to be re-configured beforehand!
		// This is the icing on the cake with much more work,
		// relaxing the filters to get rid of the trouble.
		if(rd->stage_history)
		{
			// Feed the final stage output through the filters,
			// manipulate the interpolator history to match.
			float *in = rd->stage_history
			+	rd->decim_stages*STAGE_HISTORY*rd->channels;
			// Pointer to the last 5 filtered samples
			float *last5 = NULL;
			if(rd->sflags & oversample_2x)
			{
				rd->lowpass_2x_func(rd,in, STAGE_HISTORY, rd->upbuf);
				last5 = rd->upbuf+(2*STAGE_HISTORY-5)*rd->channels;
			} else
			{
				memcpy(rd->prebuf, in, STAGE_HISTORY*sizeof(*in)*rd->channels);
				rd->lowpass_func(rd, rd->prebuf, STAGE_HISTORY);
				last5 = rd->prebuf+(STAGE_HISTORY-5)*rd->channels;
			}
			// The above wrote preemp and lowpass channel histories.
			// Now store the last samples for the interpolator history.
			for(unsigned int c=0; c<rd->channels; ++c)
				for(unsigned int i=0; i<5; ++i)
					rd->ch[c].x[i] = last5[c+i*rd->channels];
		}
	}
	else
		lpf_init(rd);

	rd->input_limit = syn123_resample_maxincount(inrate, outrate);
	return 0;
setup_resample_cleanup:
	resample_free(sh->rd);
	sh->rd = NULL;
	return err;
}

size_t attribute_align_arg
syn123_resample( syn123_handle *sh,
	float * MPG123_RESTRICT dst, float * MPG123_RESTRICT src, size_t samples )
{
	if(!sh)
		return 0;
	struct resample_data *rd = sh->rd;
	if(!rd)
		return 0;
	// Input limit is zero if no resampler configured.
	if(!samples || samples > sh->rd->input_limit)
		return 0;
	mdebug( "calling actual resample function from %p to %p with %zu samples"
	,	(void*)src, (void*)dst, samples );
	return rd->resample_func(rd, src, samples, dst);
}

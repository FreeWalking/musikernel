/* 
 * File:   pitch_core.h
 * Author: vm-user
 *
 * Created on January 7, 2012, 7:21 PM
 */

#ifndef PITCH_CORE_H
#define	PITCH_CORE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <math.h>
#include "../constants.h"
#include "interpolate-linear.h"

/*Forward declaration of functions*/

inline float f_pit_midi_note_to_hz(float);    
inline float f_pit_hz_to_midi_note(float);
inline float f_pit_midi_note_to_hz_fast(float);

/*Functions*/

inline float f_pit_midi_note_to_hz(float a_midi_note_number)
{
    float f_result;
    f_result = base_a4*pow(2,(a_midi_note_number-57)*.0833333);
    return f_result;
}

inline float f_pit_hz_to_midi_note(float _hz)
{
    float f_result=(12*log2(_hz*base_a4_recip))+57;
    return f_result;
}

/*Arrays*/

#define arr_pit_p2f_count 129

float arr_pit_p2f [arr_pit_p2f_count] = {
16.351620,
17.323936,
18.354071,
19.445461,
20.601748,
21.826790,
23.124680,
24.499743,
25.956573,
27.500031,
29.135267,
30.867739,
32.703228,
34.647865,
36.708134,
38.890911,
41.203484,
43.653568,
46.249344,
48.999474,
51.913132,
55.000046,
58.270515,
61.735462,
65.406441,
69.295708,
73.416245,
77.781799,
82.406944,
87.307114,
92.498665,
97.998917,
103.826233,
110.000061,
116.541000,
123.470886,
130.812851,
138.591385,
146.832443,
155.563553,
164.813843,
174.614182,
184.997269,
195.997787,
207.652405,
220.000061,
233.081940,
246.941711,
261.625610,
277.182678,
293.664825,
311.127014,
329.627594,
349.228271,
369.994446,
391.995453,
415.304718,
440.000000,
466.163757,
493.883270,
523.251099,
554.365234,
587.329468,
622.253906,
659.255005,
698.456360,
739.988708,
783.990662,
830.609192,
879.999756,
932.327271,
987.766296,
1046.501953,
1108.730103,
1174.658569,
1244.507446,
1318.509644,
1396.912231,
1479.976929,
1567.980957,
1661.217896,
1759.999023,
1864.653931,
1975.531982,
2093.003174,
2217.459717,
2349.316650,
2489.014160,
2637.018555,
2793.823730,
2959.953125,
3135.960938,
3322.434814,
3519.997070,
3729.306885,
3951.062988,
4186.005371,
4434.917969,
4698.631836,
4978.026855,
5274.035645,
5587.645996,
5919.904785,
6271.920410,
6644.868164,
7039.992188,
7458.611816,
7902.123535,
8372.007812,
8869.833984,
9397.260742,
9956.050781,
10548.068359,
11175.289062,
11839.805664,
12543.836914,
13289.732422,
14079.980469,
14917.219727,
15804.243164,
16744.011719,
17739.662109,
18794.517578,
19912.095703,
21096.130859,
22350.572266,
23679.605469,
25087.667969,
26579.457031};

inline float f_pit_midi_note_to_hz_fast(float a_midi_note_number)
{
    if(a_midi_note_number > arr_pit_p2f_count)
        a_midi_note_number = arr_pit_p2f_count;
    
    if(a_midi_note_number < 0)
        a_midi_note_number = 0;
    
    return f_linear_interpolate_arr(arr_pit_p2f, a_midi_note_number);
}

#ifdef	__cplusplus
}
#endif

#endif	/* PITCH_CORE_H */


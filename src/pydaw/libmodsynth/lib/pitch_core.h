/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PITCH_CORE_H
#define	PITCH_CORE_H

#include <math.h>
#include "../constants.h"
#include "interpolate-linear.h"
#include "lmalloc.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct st_pit_ratio
{
    float pitch, hz, hz_recip;
}t_pit_ratio;

t_pit_ratio * g_pit_ratio();


inline float f_pit_midi_note_to_hz(float);
inline float f_pit_hz_to_midi_note(float);
inline float f_pit_midi_note_to_samples(float, float);
float f_pit_midi_note_to_hz_fast(float);
float f_pit_midi_note_to_ratio_fast(float, float,
        t_pit_ratio *);

#ifdef	__cplusplus
}
#endif

void g_pit_ratio_init(t_pit_ratio * f_result)
{
    f_result->hz = 1.0f;
    f_result->hz_recip = 1.0f;
    //ensures that it won't coincidentally be the pitch
    f_result->pitch = 12.345432f;
}

t_pit_ratio * g_pit_ratio()
{
    t_pit_ratio * f_result = (t_pit_ratio*)malloc(sizeof(t_pit_ratio));
    g_pit_ratio_init(f_result);
    return f_result;
}

/*Functions*/

/* inline float f_pit_midi_note_to_hz(
 * float a_midi_note_number)  //typical range:  20 to 124
 *
 * Convert midi note number to hz*/
inline float f_pit_midi_note_to_hz(float a_midi_note_number)
{
    return (base_a4*pow(2.0f,(a_midi_note_number-57.0f)*.0833333f));

}

/* inline float f_pit_hz_to_midi_note(
 * float _hz) //typical range:  20 to 20000
 *
 * Convert hz to midi note number*/
inline float f_pit_hz_to_midi_note(float a_hz)
{
     return ((12.0f*log2(a_hz*base_a4_recip))+57.0f);
}

/* inline float f_pit_midi_note_to_samples(
 * float a_midi_note_number, //typical range 20 to 124
 * float a_sample_rate)
 *
 * Convert a midi note number pitch to the number of samples in a single
 * wave-length at that pitch*/
inline float f_pit_midi_note_to_samples(float a_midi_note_number,
        float a_sample_rate)
{
    /*This will be used by the _fast method, as it cannot be plotted
     * without knowing the sample rate first*/
    //return ((1/(f_pit_midi_note_to_hz_fast(a_midi_note_number))) *
    //a_sample_rate);
    return (a_sample_rate / f_pit_midi_note_to_hz_fast(a_midi_note_number));
}

/*Arrays*/

#define arr_pit_p2f_count_limit 2500.0f

#define arr_pit_p2f_count 2521
#define arr_pit_p2f_count_m1 2520

float arr_pit_p2f [arr_pit_p2f_count] __attribute__((aligned(16))) = {
16.351620, 16.398912, 16.446342, 16.493912, 16.541616, 16.589458, 16.637440, 16.685560, 16.733822, 16.782221, 16.830759, 16.879436, 16.928257, 16.977221, 17.026323, 17.075567, 17.124954, 17.174484, 17.224159, 17.273975,
17.323936, 17.374043, 17.424292, 17.474691, 17.525232, 17.575920, 17.626753, 17.677734, 17.728867, 17.780142, 17.831566, 17.883141, 17.934862, 17.986738, 18.038761, 18.090933, 18.143255, 18.195730, 18.248362, 18.301140,
18.354071, 18.407156, 18.460394, 18.513790, 18.567337, 18.621037, 18.674894, 18.728907, 18.783079, 18.837404, 18.891886, 18.946526, 19.001324, 19.056286, 19.111401, 19.166676, 19.222111, 19.277704, 19.333466, 19.389381,
19.445461, 19.501701, 19.558105, 19.614677, 19.671408, 19.728302, 19.785360, 19.842585, 19.899979, 19.957535, 20.015257, 20.073145, 20.131201, 20.189430, 20.247824, 20.306385, 20.365116, 20.424017, 20.483091, 20.542334,
20.601748, 20.661333, 20.721090, 20.781025, 20.841129, 20.901407, 20.961859, 21.022486, 21.083292, 21.144270, 21.205425, 21.266756, 21.328264, 21.389956, 21.451820, 21.513865, 21.576088, 21.638491, 21.701080, 21.763844,
21.826790, 21.889919, 21.953230, 22.016729, 22.080406, 22.144268, 22.208315, 22.272547, 22.336969, 22.401573, 22.466364, 22.531342, 22.596508, 22.661867, 22.727411, 22.793144, 22.859068, 22.925182, 22.991493, 23.057989,
23.124680, 23.191561, 23.258636, 23.325911, 23.393375, 23.461035, 23.528891, 23.596941, 23.665194, 23.733641, 23.802282, 23.871124, 23.940166, 24.009413, 24.078854, 24.148495, 24.218338, 24.288383, 24.358637, 24.429089,
24.499743, 24.570602, 24.641666, 24.712942, 24.784418, 24.856100, 24.927990, 25.000088, 25.072399, 25.144915, 25.217640, 25.290575, 25.363722, 25.437086, 25.510656, 25.584438, 25.658436, 25.732645, 25.807077, 25.881718,
25.956573, 26.031645, 26.106936, 26.182449, 26.258175, 26.334120, 26.410284, 26.486670, 26.563282, 26.640108, 26.717159, 26.794432, 26.871927, 26.949654, 27.027597, 27.105768, 27.184164, 27.262789, 27.341644, 27.420723,
27.500031, 27.579567, 27.659334, 27.739338, 27.819567, 27.900028, 27.980721, 28.061647, 28.142815, 28.224211, 28.305843, 28.387711, 28.469814, 28.552162, 28.634741, 28.717560, 28.800619, 28.883917, 28.967463, 29.051243,
29.135267, 29.219534, 29.304043, 29.388803, 29.473803, 29.559050, 29.644541, 29.730280, 29.816273, 29.902510, 29.988995, 30.075729, 30.162716, 30.249962, 30.337452, 30.425194, 30.513191, 30.601442, 30.689957, 30.778719,
30.867739, 30.957016, 31.046551, 31.136353, 31.226406, 31.316721, 31.407295, 31.498133, 31.589241, 31.680605, 31.772232, 31.864126, 31.956284, 32.048717, 32.141411, 32.234371, 32.327599, 32.421101, 32.514877, 32.608917,
32.703228, 32.797817, 32.892673, 32.987816, 33.083225, 33.178909, 33.274872, 33.371109, 33.467632, 33.564430, 33.661507, 33.758865, 33.856503, 33.954433, 34.052635, 34.151123, 34.249897, 34.348957, 34.448311, 34.547943,
34.647865, 34.748074, 34.848576, 34.949371, 35.050453, 35.151829, 35.253498, 35.355457, 35.457722, 35.560276, 35.663124, 35.766270, 35.869713, 35.973465, 36.077511, 36.181854, 36.286503, 36.391453, 36.496712, 36.602268,
36.708134, 36.814301, 36.920776, 37.027569, 37.134663, 37.242065, 37.349777, 37.457802, 37.566147, 37.674797, 37.783764, 37.893044, 38.002640, 38.112560, 38.222790, 38.333340, 38.444210, 38.555401, 38.666920, 38.778751,
38.890911, 39.003391, 39.116199, 39.229343, 39.342804, 39.456593, 39.570709, 39.685158, 39.799946, 39.915058, 40.030502, 40.146278, 40.262390, 40.378849, 40.495636, 40.612759, 40.730221, 40.848022, 40.966171, 41.084656,
41.203484, 41.322655, 41.442169, 41.562038, 41.682247, 41.802803, 41.923706, 42.044960, 42.166573, 42.288528, 42.410839, 42.533501, 42.656517, 42.779900, 42.903629, 43.027718, 43.152164, 43.276970, 43.402145, 43.527676,
43.653568, 43.779827, 43.906448, 44.033447, 44.160801, 44.288525, 44.416618, 44.545082, 44.673927, 44.803135, 44.932716, 45.062672, 45.193005, 45.323723, 45.454811, 45.586277, 45.718124, 45.850353, 45.982971, 46.115967,
46.249344, 46.383110, 46.517262, 46.651810, 46.786739, 46.922058, 47.057766, 47.193871, 47.330376, 47.467266, 47.604553, 47.742237, 47.880318, 48.018810, 48.157692, 48.296978, 48.436665, 48.576756, 48.717262, 48.858162,
48.999474, 49.141190, 49.283318, 49.425869, 49.568821, 49.712185, 49.855965, 50.000160, 50.144783, 50.289814, 50.435265, 50.581139, 50.727428, 50.874157, 51.021297, 51.168865, 51.316856, 51.465279, 51.614140, 51.763420,
51.913132, 52.063278, 52.213856, 52.364883, 52.516335, 52.668224, 52.820553, 52.973324, 53.126549, 53.280201, 53.434303, 53.588848, 53.743839, 53.899292, 54.055180, 54.211521, 54.368313, 54.525562, 54.683273, 54.841431,
55.000046, 55.159119, 55.318653, 55.478661, 55.639118, 55.800041, 55.961426, 56.123280, 56.285614, 56.448406, 56.611668, 56.775406, 56.939613, 57.104309, 57.269466, 57.435104, 57.601219, 57.767818, 57.934910, 58.102470,
58.270515, 58.439049, 58.608070, 58.777592, 58.947590, 59.118080, 59.289066, 59.460545, 59.632530, 59.805004, 59.977974, 60.151443, 60.325417, 60.499905, 60.674885, 60.850372, 61.026367, 61.202869, 61.379898, 61.557423,
61.735462, 61.914017, 62.093086, 62.272686, 62.452797, 62.633423, 62.814575, 62.996250, 63.178463, 63.361191, 63.544449, 63.728233, 63.912552, 64.097412, 64.282799, 64.468719, 64.655182, 64.842178, 65.029732, 65.217819,
65.406441, 65.595612, 65.785332, 65.975609, 66.166428, 66.357796, 66.549721, 66.742203, 66.935249, 67.128845, 67.322998, 67.517708, 67.712990, 67.908844, 68.105255, 68.302231, 68.499779, 68.697899, 68.896599, 69.095863,
69.295708, 69.496132, 69.697136, 69.898720, 70.100891, 70.303635, 70.506973, 70.710907, 70.915421, 71.120529, 71.326225, 71.532516, 71.739418, 71.946907, 72.154999, 72.363693, 72.572983, 72.782890, 72.993393, 73.204521,
73.416245, 73.628586, 73.841545, 74.055107, 74.269302, 74.484108, 74.699532, 74.915588, 75.132263, 75.349571, 75.567505, 75.786064, 76.005264, 76.225090, 76.445557, 76.666656, 76.888397, 77.110786, 77.333809, 77.557487,
77.781799, 78.006767, 78.232384, 78.458656, 78.685585, 78.913162, 79.141396, 79.370300, 79.599861, 79.830093, 80.060982, 80.292534, 80.524773, 80.757668, 80.991249, 81.225494, 81.460419, 81.696030, 81.932312, 82.169289,
82.406944, 82.645287, 82.884323, 83.124046, 83.364471, 83.605583, 83.847389, 84.089905, 84.333115, 84.577034, 84.821648, 85.066978, 85.313019, 85.559769, 85.807236, 86.055412, 86.304306, 86.553925, 86.804260, 87.055328,
87.307114, 87.559631, 87.812881, 88.066856, 88.321579, 88.577026, 88.833214, 89.090149, 89.347816, 89.606247, 89.865410, 90.125320, 90.385994, 90.647415, 90.909599, 91.172531, 91.436218, 91.700691, 91.965912, 92.231903,
92.498665, 92.766190, 93.034508, 93.303581, 93.573448, 93.844086, 94.115509, 94.387718, 94.660713, 94.934509, 95.209084, 95.484451, 95.760620, 96.037582, 96.315361, 96.593925, 96.873299, 97.153496, 97.434486, 97.716301,
97.998917, 98.282356, 98.566620, 98.851700, 99.137611, 99.424347, 99.711906, 100.000305, 100.289528, 100.579605, 100.870506, 101.162247, 101.454842, 101.748276, 102.042564, 102.337700, 102.633682, 102.930534, 103.228241, 103.526810,
103.826233, 104.126526, 104.427681, 104.729736, 105.032639, 105.336418, 105.641083, 105.946617, 106.253067, 106.560379, 106.868576, 107.177666, 107.487648, 107.798553, 108.110329, 108.423012, 108.736595, 109.051094, 109.366516, 109.682831,
110.000061, 110.318207, 110.637276, 110.957291, 111.278206, 111.600052, 111.922821, 112.246529, 112.571198, 112.896782, 113.223305, 113.550774, 113.879196, 114.208588, 114.538902, 114.870178, 115.202408, 115.535606, 115.869789, 116.204910,
116.541000, 116.878067, 117.216110, 117.555153, 117.895149, 118.236130, 118.578094, 118.921051, 119.265030, 119.609970, 119.955910, 120.302856, 120.650803, 120.999779, 121.349739, 121.700714, 122.052696, 122.405708, 122.759758, 123.114807,
123.470886, 123.827995, 124.186134, 124.545341, 124.905556, 125.266815, 125.629112, 125.992462, 126.356895, 126.722351, 127.088860, 127.456429, 127.825066, 128.194794, 128.565567, 128.937408, 129.310333, 129.684326, 130.059433, 130.435593,
130.812851, 131.191193, 131.570633, 131.951187, 132.332825, 132.715561, 133.099411, 133.484360, 133.870468, 134.257645, 134.645950, 135.035385, 135.425934, 135.817657, 136.210464, 136.604416, 136.999512, 137.395752, 137.793167, 138.191696,
138.591385, 138.992218, 139.394226, 139.797409, 140.201736, 140.607239, 141.013901, 141.421753, 141.830811, 142.241013, 142.652420, 143.065002, 143.478775, 143.893784, 144.309967, 144.727341, 145.145935, 145.565720, 145.986771, 146.408997,
146.832443, 147.257126, 147.683029, 148.110199, 148.538559, 148.968170, 149.399033, 149.831131, 150.264511, 150.699112, 151.134964, 151.572083, 152.010468, 152.450150, 152.891083, 153.333267, 153.776749, 154.221512, 154.667587, 155.114929,
155.563553, 156.013489, 156.464706, 156.917282, 157.371124, 157.826279, 158.282745, 158.740540, 159.199692, 159.660141, 160.121918, 160.585022, 161.049484, 161.515305, 161.982452, 162.450943, 162.920792, 163.391998, 163.864594, 164.338531,
164.813843, 165.290527, 165.768585, 166.248062, 166.728897, 167.211121, 167.694733, 168.179749, 168.666199, 169.154022, 169.643250, 170.133911, 170.625977, 171.119507, 171.614426, 172.110779, 172.608551, 173.107788, 173.608490, 174.110611,
174.614182, 175.119202, 175.625687, 176.133682, 176.643112, 177.154007, 177.666367, 178.180222, 178.695602, 179.212433, 179.730759, 180.250595, 180.771912, 181.294800, 181.819138, 182.345001, 182.872391, 183.401306, 183.931793, 184.463760,
184.997269, 185.532333, 186.068939, 186.607132, 187.146851, 187.688126, 188.230957, 188.775375, 189.321396, 189.868958, 190.418106, 190.968842, 191.521164, 192.075134, 192.630661, 193.187805, 193.746552, 194.306915, 194.868942, 195.432541,
195.997787, 196.564651, 197.133163, 197.703369, 198.275177, 198.848633, 199.423752, 200.000534, 200.579025, 201.159149, 201.740952, 202.324432, 202.909607, 203.496521, 204.085083, 204.675339, 205.267319, 205.860992, 206.456436, 207.053558,
207.652405, 208.252991, 208.855316, 209.459412, 210.065231, 210.672791, 211.282104, 211.893173, 212.506073, 213.120697, 213.737091, 214.355270, 214.975235, 215.597046, 216.220596, 216.845963, 217.473129, 218.102127, 218.732971, 219.365601,
220.000061, 220.636353, 221.274490, 221.914520, 222.556351, 223.200027, 223.845581, 224.492996, 225.142334, 225.793503, 226.446548, 227.101486, 227.758316, 228.417099, 229.077744, 229.740295, 230.404755, 231.071136, 231.739502, 232.409760,
233.081940, 233.756073, 234.432144, 235.110229, 235.790237, 236.472198, 237.156128, 237.842041, 238.529984, 239.219879, 239.911758, 240.605637, 241.301529, 241.999481, 242.699402, 243.401352, 244.105331, 244.811340, 245.519455, 246.229553,
246.941711, 247.655930, 248.372208, 249.090607, 249.811035, 250.533554, 251.258163, 251.984863, 252.713715, 253.444626, 254.177643, 254.912796, 255.650055, 256.389526, 257.131073, 257.874756, 258.620575, 259.368591, 260.118805, 260.871124,
261.625610, 262.382294, 263.141174, 263.902313, 264.665588, 265.431061, 266.198730, 266.968658, 267.740845, 268.515228, 269.291840, 270.070679, 270.851807, 271.635223, 272.420868, 273.208771, 273.998962, 274.791443, 275.586243, 276.383301,
277.182678, 277.984375, 278.788361, 279.594757, 280.403412, 281.214386, 282.027740, 282.843414, 283.661530, 284.481964, 285.304749, 286.129913, 286.957489, 287.787506, 288.619843, 289.454590, 290.291779, 291.131378, 291.973450, 292.817902,
293.664825, 294.514160, 295.365967, 296.220306, 297.077057, 297.936279, 298.797974, 299.662170, 300.528931, 301.398132, 302.269836, 303.144073, 304.020844, 304.900208, 305.782074, 306.666473, 307.553406, 308.442932, 309.335083, 310.229767,
311.127014, 312.026886, 312.929352, 313.834473, 314.742157, 315.652466, 316.565430, 317.480988, 318.399292, 319.320190, 320.243744, 321.169952, 322.098877, 323.030518, 323.964813, 324.901794, 325.841492, 326.783905, 327.729095, 328.676971,
329.627594, 330.580963, 331.537079, 332.496033, 333.457703, 334.422150, 335.389374, 336.359406, 337.332306, 338.307953, 339.286407, 340.267731, 341.251862, 342.238922, 343.228760, 344.221436, 345.217010, 346.215485, 347.216888, 348.221130,
349.228271, 350.238312, 351.251282, 352.267273, 353.286102, 354.307892, 355.332642, 356.360352, 357.391113, 358.424774, 359.461426, 360.501068, 361.543732, 362.589478, 363.638184, 364.689911, 365.744690, 366.802521, 367.863464, 368.927429,
369.994446, 371.064575, 372.137756, 373.214172, 374.293579, 375.376129, 376.461823, 377.550629, 378.642700, 379.737823, 380.836121, 381.937592, 383.042236, 384.150177, 385.261230, 386.375488, 387.492981, 388.613708, 389.737762, 390.864990,
391.995453, 393.129211, 394.266235, 395.406616, 396.550232, 397.697144, 398.847382, 400.000946, 401.157959, 402.318176, 403.481781, 404.648773, 405.819092, 406.992920, 408.170044, 409.350555, 410.534515, 411.721863, 412.912750, 414.106995,
415.304718, 416.505859, 417.710510, 418.918732, 420.130341, 421.345459, 422.564087, 423.786255, 425.012024, 426.241272, 427.474060, 428.710419, 429.950348, 431.193970, 432.441071, 433.691803, 434.946167, 436.204132, 437.465820, 438.731079,
440.000000, 441.272583, 442.548859, 443.828918, 445.112579, 446.399933, 447.691040, 448.985870, 450.284546, 451.586884, 452.892975, 454.202850, 455.516510, 456.834076, 458.155365, 459.480469, 460.809387, 462.142151, 463.478882, 464.819366,
466.163757, 467.512024, 468.864166, 470.220337, 471.580322, 472.944244, 474.312134, 475.683960, 477.059845, 478.439636, 479.823395, 481.211151, 482.602936, 483.998840, 485.398682, 486.802582, 488.210541, 489.622559, 491.038757, 492.458954,
493.883270, 495.311707, 496.744263, 498.181091, 499.621948, 501.066986, 502.516174, 503.969574, 505.427307, 506.889099, 508.355164, 509.825439, 511.299988, 512.778870, 514.261963, 515.749329, 517.241028, 518.737000, 520.237427, 521.742065,
523.251099, 524.764465, 526.282227, 527.804443, 529.330994, 530.861938, 532.397339, 533.937134, 535.481567, 537.030273, 538.583496, 540.141235, 541.703430, 543.270325, 544.841553, 546.417419, 547.997742, 549.582703, 551.172363, 552.766479,
554.365234, 555.968567, 557.576538, 559.189331, 560.806641, 562.428650, 564.055298, 565.686707, 567.322937, 568.963745, 570.609375, 572.259705, 573.914795, 575.574829, 577.239502, 578.909058, 580.583374, 582.262573, 583.946777, 585.635681,
587.329468, 589.028137, 590.731750, 592.440430, 594.153931, 595.872375, 597.595764, 599.324158, 601.057678, 602.796082, 604.539551, 606.288025, 608.041565, 609.800293, 611.563965, 613.332764, 615.106689, 616.885681, 618.670044, 620.459351,
622.253906, 624.053589, 625.858521, 627.668762, 629.484131, 631.304749, 633.130676, 634.961853, 636.798401, 638.640198, 640.487305, 642.339722, 644.197571, 646.060852, 647.929443, 649.803406, 651.682800, 653.567627, 655.458008, 657.353760,
659.255005, 661.161865, 663.073975, 664.991882, 666.915039, 668.844116, 670.778687, 672.718628, 674.664429, 676.615540, 678.572632, 680.535400, 682.503540, 684.477661, 686.457153, 688.442688, 690.434021, 692.430725, 694.433594, 696.441895,
698.456360, 700.476624, 702.502380, 704.534363, 706.571899, 708.615601, 710.665283, 712.720520, 714.782043, 716.849182, 718.922668, 721.002136, 723.087280, 725.178772, 727.276001, 729.379639, 731.489319, 733.604797, 735.726746, 737.854492,
739.988708, 742.129089, 744.275330, 746.428101, 748.586792, 750.752075, 752.923584, 755.101074, 757.285156, 759.475281, 761.671997, 763.875122, 766.084229, 768.300110, 770.522095, 772.750793, 774.985901, 777.227234, 779.475281, 781.729553,
783.990662, 786.258362, 788.532227, 790.813049, 793.100098, 795.394104, 797.694763, 800.001709, 802.315674, 804.635986, 806.963379, 809.297485, 811.638000, 813.985596, 816.339661, 818.700928, 821.068970, 823.443542, 825.825317, 828.213623,
830.609192, 833.011719, 835.420776, 837.837219, 840.260254, 842.690674, 845.128113, 847.572266, 850.023804, 852.482117, 854.947876, 857.420776, 859.900452, 862.387695, 864.881714, 867.383362, 869.892273, 872.408020, 874.931396, 877.461731,
879.999756, 882.545105, 885.097473, 887.657593, 890.224670, 892.799622, 895.382019, 897.971497, 900.568848, 903.173340, 905.785706, 908.405640, 911.032776, 913.667908, 916.310242, 918.960632, 921.618713, 924.284058, 926.957520, 929.638306,
932.327271, 935.023987, 937.728088, 940.440430, 943.160217, 945.888245, 948.624207, 951.367615, 954.119446, 956.878784, 959.646484, 962.422241, 965.205566, 967.997437, 970.796875, 973.604858, 976.421021, 979.244812, 982.077271, 984.917419,
987.766296, 990.623352, 993.488281, 996.361877, 999.243408, 1002.133667, 1005.032288, 1007.938904, 1010.854309, 1013.777710, 1016.710022, 1019.650818, 1022.599670, 1025.557495, 1028.523438, 1031.498413, 1034.481934, 1037.473755, 1040.474609, 1043.483643,
1046.501953, 1049.528809, 1052.564087, 1055.608643, 1058.661499, 1061.723633, 1064.794556, 1067.874023, 1070.962769, 1074.060059, 1077.166748, 1080.282349, 1083.406616, 1086.540283, 1089.682617, 1092.834473, 1095.995483, 1099.165161, 1102.344360, 1105.532349,
1108.730103, 1111.937134, 1115.152832, 1118.378418, 1121.612793, 1124.856934, 1128.110596, 1131.373047, 1134.645508, 1137.927002, 1141.218384, 1144.519287, 1147.829224, 1151.149292, 1154.478516, 1157.817749, 1161.166748, 1164.524780, 1167.893188, 1171.270752,
1174.658569, 1178.056274, 1181.463257, 1184.880615, 1188.307251, 1191.744385, 1195.191528, 1198.647949, 1202.114990, 1205.591553, 1209.078735, 1212.575928, 1216.082764, 1219.600220, 1223.127319, 1226.665161, 1230.213257, 1233.770996, 1237.339722, 1240.918091,
1244.507446, 1248.107056, 1251.716675, 1255.337158, 1258.967651, 1262.609131, 1266.261230, 1269.923340, 1273.596558, 1277.279785, 1280.974243, 1284.679443, 1288.394775, 1292.121338, 1295.858276, 1299.606445, 1303.365479, 1307.134888, 1310.915771, 1314.706909,
1318.509644, 1322.323364, 1326.147583, 1329.983398, 1333.829712, 1337.687866, 1341.557007, 1345.436890, 1349.328491, 1353.230713, 1357.144897, 1361.070435, 1365.006592, 1368.954834, 1372.913940, 1376.885010, 1380.867676, 1384.861084, 1388.866821, 1392.883423,
1396.912231, 1400.952759, 1405.004395, 1409.068359, 1413.143311, 1417.230835, 1421.330078, 1425.440674, 1429.563721, 1433.697998, 1437.844971, 1442.003784, 1446.174194, 1450.357178, 1454.551636, 1458.758789, 1462.978271, 1467.209229, 1471.453125, 1475.708496,
1479.976929, 1484.257812, 1488.550293, 1492.855835, 1497.173218, 1501.503784, 1505.846802, 1510.201660, 1514.569946, 1518.950073, 1523.343628, 1527.749756, 1532.168091, 1536.599854, 1541.043701, 1545.501099, 1549.971436, 1554.453979, 1558.950195, 1563.458740,
1567.980957, 1572.516235, 1577.063965, 1581.625610, 1586.199707, 1590.787720, 1595.389038, 1600.002930, 1604.630859, 1609.271484, 1613.926270, 1618.594482, 1623.275513, 1627.970825, 1632.678955, 1637.401367, 1642.137451, 1646.886597, 1651.650146, 1656.426758,
1661.217896, 1666.022949, 1670.841064, 1675.673950, 1680.520020, 1685.380859, 1690.255737, 1695.144043, 1700.047119, 1704.963745, 1709.895264, 1714.841064, 1719.800415, 1724.774902, 1729.763062, 1734.766235, 1739.784058, 1744.815552, 1749.862305, 1754.922974,
1759.999023, 1765.089722, 1770.194458, 1775.314697, 1780.448853, 1785.598755, 1790.763550, 1795.942505, 1801.137207, 1806.346069, 1811.570923, 1816.810791, 1822.065063, 1827.335327, 1832.619995, 1837.920776, 1843.236938, 1848.567627, 1853.914551, 1859.276123,
1864.653931, 1870.047363, 1875.455688, 1880.880371, 1886.319824, 1891.776001, 1897.247925, 1902.734741, 1908.238281, 1913.756958, 1919.292480, 1924.843994, 1930.410645, 1935.994263, 1941.593262, 1947.209229, 1952.841431, 1958.489136, 1964.153931, 1969.834351,
1975.531982, 1981.246216, 1986.975952, 1992.723267, 1998.486206, 2004.266724, 2010.064087, 2015.877197, 2021.708008, 2027.554932, 2033.419556, 2039.301147, 2045.198853, 2051.114502, 2057.046387, 2062.996338, 2068.963379, 2074.946777, 2080.948486, 2086.966797,
2093.003174, 2099.057129, 2105.127686, 2111.216797, 2117.322266, 2123.446533, 2129.588623, 2135.747559, 2141.925049, 2148.119629, 2154.332764, 2160.564209, 2166.812500, 2173.080078, 2179.364502, 2185.668457, 2191.990234, 2198.329590, 2204.688232, 2211.064209,
2217.459717, 2223.873535, 2230.304932, 2236.756104, 2243.224854, 2249.713379, 2256.220459, 2262.745605, 2269.290527, 2275.853271, 2282.436035, 2289.038086, 2295.657959, 2302.298096, 2308.956299, 2315.635010, 2322.332764, 2329.049072, 2335.785645, 2342.540771,
2349.316650, 2356.111816, 2362.925781, 2369.760498, 2376.613770, 2383.488037, 2390.382324, 2397.295410, 2404.229492, 2411.182617, 2418.156738, 2425.151123, 2432.164795, 2439.199707, 2446.253906, 2453.329590, 2460.425781, 2467.541504, 2474.678711, 2481.835449,
2489.014160, 2496.213623, 2503.432617, 2510.673828, 2517.934570, 2525.217773, 2532.521729, 2539.845947, 2547.192383, 2554.558838, 2561.947754, 2569.358154, 2576.788818, 2584.241943, 2591.715820, 2599.212158, 2606.730225, 2614.269043, 2621.830811, 2629.413086,
2637.018555, 2644.645996, 2652.294434, 2659.966064, 2667.658691, 2675.374756, 2683.113281, 2690.872803, 2698.656250, 2706.460693, 2714.289062, 2722.140137, 2730.012451, 2737.908936, 2745.827148, 2753.769287, 2761.734375, 2769.721436, 2777.732910, 2785.766113,
2793.823730, 2801.904785, 2810.008057, 2818.135742, 2826.285889, 2834.460938, 2842.659424, 2850.880371, 2859.126465, 2867.395264, 2875.688965, 2884.006836, 2892.347412, 2900.713379, 2909.102295, 2917.516846, 2925.955811, 2934.417725, 2942.905273, 2951.416260,
2959.953125, 2968.514648, 2977.099609, 2985.710938, 2994.345703, 3003.006592, 3011.692627, 3020.402588, 3029.138916, 3037.899414, 3046.686279, 3055.498779, 3064.335449, 3073.198730, 3082.086670, 3091.001465, 3099.941895, 3108.906982, 3117.899414, 3126.916504,
3135.960938, 3145.031738, 3154.127197, 3163.250488, 3172.398682, 3181.574707, 3190.777100, 3200.004883, 3209.260986, 3218.542236, 3227.851562, 3237.187988, 3246.550049, 3255.940674, 3265.356934, 3274.801758, 3284.274170, 3293.772217, 3303.299316, 3312.852539,
3322.434814, 3332.044922, 3341.681152, 3351.346924, 3361.039062, 3370.760742, 3380.510498, 3390.287109, 3400.093506, 3409.926514, 3419.789551, 3429.681152, 3439.599854, 3449.548828, 3459.525146, 3469.531494, 3479.567139, 3489.630127, 3499.723633, 3509.844971,
3519.997070, 3530.178467, 3540.387939, 3550.628418, 3560.896729, 3571.196533, 3581.526123, 3591.884033, 3602.273438, 3612.691162, 3623.140869, 3633.620605, 3644.129150, 3654.669678, 3665.239014, 3675.840576, 3686.472900, 3697.134277, 3707.828125, 3718.551025,
3729.306885, 3740.093750, 3750.910156, 3761.759521, 3772.638672, 3783.550781, 3794.494629, 3805.468506, 3816.475586, 3827.512939, 3838.583984, 3849.686768, 3860.820312, 3871.987549, 3883.185303, 3894.417480, 3905.681885, 3916.977051, 3928.306885, 3939.667480,
3951.062988, 3962.491211, 3973.950928, 3985.445312, 3996.971436, 4008.532471, 4020.126953, 4031.753174, 4043.415039, 4055.108643, 4066.837891, 4078.601074, 4090.396484, 4102.227539, 4114.091309, 4125.991211, 4137.925781, 4149.892578, 4161.895996, 4173.932129,
4186.005371, 4198.113281, 4210.254395, 4222.432129, 4234.643555, 4246.892090, 4259.176270, 4271.493652, 4283.848633, 4296.237793, 4308.664551, 4321.126953, 4333.624023, 4346.158691, 4358.728027, 4371.335449, 4383.979492, 4396.658203, 4409.375000, 4422.126953,
4434.917969, 4447.745605, 4460.608887, 4473.510742, 4486.448242, 4499.425293, 4512.439941, 4525.489746, 4538.579590, 4551.705078, 4564.871094, 4578.074707, 4591.314453, 4604.594727, 4617.911133, 4631.268555, 4644.664062, 4658.096680, 4671.570312, 4685.080566,
4698.631836, 4712.222656, 4725.850098, 4739.519531, 4753.226562, 4766.975098, 4780.763184, 4794.589355, 4808.457520, 4822.363770, 4836.312012, 4850.301270, 4864.328125, 4878.397949, 4892.506348, 4906.657715, 4920.850098, 4935.081543, 4949.355957, 4963.669434,
4978.026855, 4992.425781, 5006.863770, 5021.346191, 5035.867676, 5050.434082, 5065.041992, 5079.690430, 5094.383301, 5109.116211, 5123.894043, 5138.714844, 5153.576172, 5168.482422, 5183.430176, 5198.422852, 5213.458984, 5228.536621, 5243.660156, 5258.824707,
5274.035645, 5289.290527, 5304.587402, 5319.930664, 5335.315918, 5350.748047, 5366.225098, 5381.744141, 5397.311035, 5412.919922, 5428.576660, 5444.278809, 5460.023438, 5475.816406, 5491.652832, 5507.537109, 5523.467285, 5539.441406, 5555.463867, 5571.530762,
5587.645996, 5603.808105, 5620.014648, 5636.270020, 5652.570312, 5668.920410, 5685.317383, 5701.759277, 5718.251465, 5734.788574, 5751.376465, 5768.012207, 5784.693359, 5801.425293, 5818.203125, 5835.032227, 5851.909668, 5868.833496, 5885.809082, 5902.831055,
5919.904785, 5937.027832, 5954.197754, 5971.419922, 5988.689453, 6006.011719, 6023.383789, 6040.803711, 6058.276367, 6075.796875, 6093.371094, 6110.995605, 6128.668945, 6146.395996, 6164.171387, 6182.000977, 6199.882324, 6217.812500, 6235.797363, 6253.831543,
6271.920410, 6290.061523, 6308.252441, 6326.499023, 6344.795410, 6363.147461, 6381.552734, 6400.008301, 6418.520020, 6437.082520, 6455.701660, 6474.374512, 6493.098633, 6511.879395, 6530.711914, 6549.602051, 6568.546387, 6587.542480, 6606.596680, 6625.703125,
6644.868164, 6664.087891, 6683.360840, 6702.691895, 6722.076172, 6741.519531, 6761.019531, 6780.572266, 6800.185059, 6819.851074, 6839.577148, 6859.360352, 6879.198242, 6899.095703, 6919.048340, 6939.061035, 6959.132324, 6979.258301, 6999.445312, 7019.687988,
7039.992188, 7060.354980, 7080.773926, 7101.254883, 7121.791504, 7142.391113, 7163.050293, 7183.766113, 7204.544922, 7225.380371, 7246.279785, 7267.239258, 7288.256348, 7309.337402, 7330.476074, 7351.679199, 7372.943848, 7394.266113, 7415.653809, 7437.100098,
7458.611816, 7480.185547, 7501.818359, 7523.517090, 7545.275391, 7567.099609, 7588.987305, 7610.934570, 7632.949219, 7655.023926, 7677.165527, 7699.371582, 7721.638184, 7743.972656, 7766.368652, 7788.832520, 7811.361328, 7833.952148, 7856.611328, 7879.333008,
7902.123535, 7924.980469, 7947.899414, 7970.888672, 7993.940430, 8017.062500, 8040.251953, 8063.504395, 8086.827637, 8110.214844, 8133.673340, 8157.199707, 8180.790527, 8204.453125, 8228.180664, 8251.980469, 8275.848633, 8299.783203, 8323.790039, 8347.862305,
8372.007812, 8396.223633, 8420.505859, 8444.862305, 8469.285156, 8493.782227, 8518.349609, 8542.985352, 8567.695312, 8592.473633, 8617.327148, 8642.251953, 8667.245117, 8692.315430, 8717.454102, 8742.668945, 8767.956055, 8793.313477, 8818.748047, 8844.251953,
8869.833984, 8895.489258, 8921.214844, 8947.019531, 8972.894531, 8998.848633, 9024.876953, 9050.977539, 9077.157227, 9103.408203, 9129.739258, 9156.146484, 9182.626953, 9209.187500, 9235.820312, 9262.534180, 9289.326172, 9316.191406, 9343.137695, 9370.158203,
9397.260742, 9424.442383, 9451.698242, 9479.037109, 9506.450195, 9533.947266, 9561.523438, 9589.175781, 9616.912109, 9644.724609, 9672.622070, 9700.599609, 9728.653320, 9756.793945, 9785.010742, 9813.313477, 9841.698242, 9870.160156, 9898.708984, 9927.336914,
9956.050781, 9984.848633, 10013.724609, 10042.689453, 10071.733398, 10100.865234, 10130.081055, 10159.377930, 10188.763672, 10218.229492, 10247.785156, 10277.426758, 10307.149414, 10336.962891, 10366.857422, 10396.842773, 10426.915039, 10457.070312, 10487.317383, 10517.646484,
10548.068359, 10578.578125, 10609.171875, 10639.858398, 10670.628906, 10701.493164, 10732.447266, 10763.485352, 10794.619141, 10825.836914, 10857.150391, 10888.554688, 10920.043945, 10951.629883, 10983.302734, 11015.071289, 11046.931641, 11078.879883, 11110.924805, 11143.057617,
11175.289062, 11207.613281, 11240.025391, 11272.537109, 11305.137695, 11337.836914, 11370.631836, 11403.515625, 11436.500000, 11469.574219, 11502.750000, 11536.021484, 11569.383789, 11602.847656, 11636.403320, 11670.061523, 11703.816406, 11737.664062, 11771.614258, 11805.658203,
11839.805664, 11874.051758, 11908.392578, 11942.836914, 11977.375977, 12012.019531, 12046.763672, 12081.603516, 12116.548828, 12151.590820, 12186.738281, 12221.988281, 12257.334961, 12292.788086, 12328.339844, 12363.999023, 12399.760742, 12435.621094, 12471.590820, 12507.659180,
12543.836914, 12580.120117, 12616.501953, 12652.994141, 12689.586914, 12726.291016, 12763.101562, 12800.012695, 12837.036133, 12874.161133, 12911.399414, 12948.745117, 12986.193359, 13023.754883, 13061.419922, 13099.200195, 13137.088867, 13175.082031, 13213.190430, 13251.403320,
13289.732422, 13328.171875, 13366.717773, 13405.380859, 13444.149414, 13483.036133, 13522.035156, 13561.140625, 13600.366211, 13639.698242, 13679.151367, 13718.717773, 13758.392578, 13798.187500, 13838.092773, 13878.119141, 13918.260742, 13958.512695, 13998.886719, 14039.372070,
14079.980469, 14120.706055, 14161.543945, 14202.505859, 14243.579102, 14284.778320, 14326.096680, 14367.528320, 14409.085938, 14450.756836, 14492.555664, 14534.474609, 14576.508789, 14618.670898, 14660.948242, 14703.354492, 14745.882812, 14788.528320, 14831.303711, 14874.196289,
14917.219727, 14960.367188, 15003.632812, 15047.030273, 15090.545898, 15134.195312, 15177.970703, 15221.865234, 15265.893555, 15310.042969, 15354.327148, 15398.739258, 15443.272461, 15487.941406, 15532.733398, 15577.661133, 15622.718750, 15667.899414, 15713.218750, 15758.662109,
15804.243164, 15849.956055, 15895.794922, 15941.772461, 15987.876953, 16034.121094, 16080.499023, 16127.003906, 16173.651367, 16220.425781, 16267.342773, 16314.395508, 16361.577148, 16408.902344, 16456.357422, 16503.957031, 16551.693359, 16599.560547, 16647.574219, 16695.720703,
16744.011719, 16792.443359, 16841.007812, 16889.718750, 16938.564453, 16987.558594, 17036.695312, 17085.964844, 17135.386719, 17184.941406, 17234.648438, 17284.500000, 17334.486328, 17384.625000, 17434.902344, 17485.332031, 17535.908203, 17586.623047, 17637.490234, 17688.498047,
17739.662109, 17790.972656, 17842.425781, 17894.033203, 17945.783203, 17997.691406, 18049.750000, 18101.949219, 18154.308594, 18206.810547, 18259.474609, 18312.289062, 18365.248047, 18418.369141, 18471.634766, 18525.064453, 18578.646484, 18632.376953, 18686.269531, 18740.310547,
18794.517578, 18848.878906, 18903.390625, 18958.068359, 19012.894531, 19067.888672, 19123.042969, 19178.347656, 19233.820312, 19289.443359, 19345.238281, 19401.193359, 19457.302734, 19513.582031, 19570.015625, 19626.621094, 19683.390625, 19740.314453, 19797.414062, 19854.667969,
19912.095703, 19969.691406, 20027.445312, 20085.373047, 20143.460938, 20201.724609, 20260.158203, 20318.750000, 20377.521484, 20436.453125, 20495.564453, 20554.847656, 20614.292969, 20673.919922, 20733.708984, 20793.679688, 20853.824219, 20914.134766, 20974.628906, 21035.287109,
21096.130859, 21157.150391, 21218.337891, 21279.710938, 21341.251953, 21402.980469, 21464.888672, 21526.964844, 21589.230469, 21651.667969, 21714.294922, 21777.101562, 21840.082031, 21903.253906, 21966.597656, 22030.136719, 22093.857422, 22157.753906, 22221.843750, 22286.109375,
22350.572266, 22415.220703, 22480.044922, 22545.068359, 22610.269531, 22675.667969, 22741.255859, 22807.025391, 22872.994141, 22939.142578, 23005.494141, 23072.035156, 23138.759766, 23205.689453, 23272.800781, 23340.115234, 23407.626953, 23475.322266, 23543.222656, 23611.310547,
23679.605469};

/* float f_pit_midi_note_to_hz_fast(
 * float a_midi_note_number) //range: 20 to 124
 *
 * Convert midi note number to hz, using a fast table lookup.
 * You should prefer this function whenever possible, it is much faster than the
 * regular version.
 */
float f_pit_midi_note_to_hz_fast(float a_midi_note_number)
{
    float arr_index = (a_midi_note_number * 20.0f) - 1.0f;

    if((arr_index) > arr_pit_p2f_count_limit)
    {
        arr_index = arr_pit_p2f_count_limit ;
    }
    else if(arr_index < 0.0f)
    {
        arr_index = 0.0f;
    }

    return f_linear_interpolate_arr(arr_pit_p2f, arr_index);
}


/* inline float f_pit_midi_note_to_ratio_fast(
 * float a_base_pitch, //The base pitch of the sample in MIDI note number
 * float a_transposed_pitch, //The pitch the sample will be transposed to
 * t_pit_pitch_core* a_pit,
 * t_pit_ratio * a_ratio)
 */
float f_pit_midi_note_to_ratio_fast(float a_base_pitch,
        float a_transposed_pitch, t_pit_ratio *__restrict a_ratio)
{
    if(a_base_pitch != (a_ratio->pitch))
    {
        a_ratio->pitch = a_base_pitch;
        a_ratio->hz = f_pit_midi_note_to_hz_fast(a_base_pitch);
        a_ratio->hz_recip = 1.0f / (a_ratio->hz);
    }

    return a_ratio->hz_recip * f_pit_midi_note_to_hz_fast(a_transposed_pitch);
}


#endif	/* PITCH_CORE_H */


#ifndef STATIC_SUPER_SAMPLING_INCLUDED
#define STATIC_SUPER_SAMPLING_INCLUDED

#if !defined(SUPER_SAMPLING) || SUPER_SAMPLING == 1
    #define NUM_SAMPLES 1
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(0.0, 0.0)
    );

#elif SUPER_SAMPLING == 2
    #define NUM_SAMPLES 2
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.25, -0.25),
        vec2( 0.25,  0.25)
    );

#elif SUPER_SAMPLING == 4
    #define NUM_SAMPLES 4
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.25, -0.25),
        vec2( 0.25, -0.25),
        vec2(-0.25,  0.25),
        vec2( 0.25,  0.25)
    );

#elif SUPER_SAMPLING == 6
    #define NUM_SAMPLES 6
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        dvec2(-0.33, -0.25),
        dvec2( 0.0,  -0.25),
        dvec2( 0.33, -0.25),
        dvec2(-0.33,  0.25),
        dvec2( 0.0,   0.25),
        dvec2( 0.33,  0.25)
    );

#elif SUPER_SAMPLING == 8
    #define NUM_SAMPLES 8
    const double ssRadius = 0.35;
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2( ssRadius,  0),
        vec2(-ssRadius,  0),
        vec2( 0,  ssRadius),
        vec2( 0, -ssRadius),
        vec2( ssRadius/sqrt(2),  ssRadius/sqrt(2)),
        vec2(-ssRadius/sqrt(2),  ssRadius/sqrt(2)),
        vec2( ssRadius/sqrt(2), -ssRadius/sqrt(2)),
        vec2(-ssRadius/sqrt(2), -ssRadius/sqrt(2))
    );


#elif SUPER_SAMPLING == 12
    #define NUM_SAMPLES 12
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.25, -0.375), vec2( 0.0, -0.375), vec2(0.25, -0.375),
        vec2(-0.25, -0.125), vec2( 0.0, -0.125), vec2(0.25, -0.125),
        vec2(-0.25,  0.125), vec2( 0.0,  0.125), vec2(0.25,  0.125),
        vec2(-0.25,  0.375), vec2( 0.0,  0.375), vec2(0.25,  0.375)
    );

#elif SUPER_SAMPLING == 16
    #define NUM_SAMPLES 16
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.375, -0.375),
        vec2(+0.375, +0.375),
        vec2(-0.375, +0.125),
        vec2(-0.125, +0.375),
        vec2(+0.125, -0.375),
        vec2(+0.375, -0.125),
        vec2(-0.125, -0.125),
        vec2(+0.125, +0.125),
        vec2(-0.500,  0.000),
        vec2(+0.500,  0.000),
        vec2( 0.000, -0.500),
        vec2( 0.000, +0.500),
        vec2(-0.250, +0.250),
        vec2(+0.250, -0.250),
        vec2(-0.250, -0.250),
        vec2(+0.250, +0.250)
    );

#elif SUPER_SAMPLING == 1601
    #define NUM_SAMPLES 16
    // pmj02bn samples from https://github.com/Andrew-Helmer/pmj-cpp/blob/master/sample_sequences/pmj02bn/1024_samples_00.txt
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.026716370187168492, -0.004929086373079206),
        vec2(0.4484750221428714, 0.4956808432839589),
        vec2(-0.13006748618511488, 0.28351616863133655),
        vec2(0.36038210492843936, -0.211598426880599),
        vec2(-0.2727249773071365, -0.2730566294647252),
        vec2(0.24270376568844954, 0.21479654945808768),
        vec2(-0.11750484798082572, 0.1268997253162244),
        vec2(0.4249501577365884, -0.3510844866084426),
        vec2(-0.43578090240216727, -0.12889371382781623),
        vec2(0.07586719938445674, 0.3731150628855976),
        vec2(-0.3197436297238845, 0.43049516646016384),
        vec2(0.18642756070396505, -0.06874105769335287),
        vec2(-0.1888918962307623, -0.44509281923105454),
        vec2(0.3086305612034951, 0.05419428038956031),
        vec2(-0.3117787331141107, 0.23123407093109194),
        vec2(0.19021486592645764, -0.28258149109763075)
    );

#elif SUPER_SAMPLING == 32
    #define NUM_SAMPLES 32
    // pmj02bn samples from https://github.com/Andrew-Helmer/pmj-cpp/blob/master/sample_sequences/pmj02bn/1024_samples_00.txt
    const vec2 sample_offsets[NUM_SAMPLES] = vec2[](
        vec2(-0.026716370187168492, -0.004929086373079206),
        vec2(0.4484750221428714, 0.4956808432839589),
        vec2(-0.13006748618511488, 0.28351616863133655),
        vec2(0.36038210492843936, -0.211598426880599),
        vec2(-0.2727249773071365, -0.2730566294647252),
        vec2(0.24270376568844954, 0.21479654945808768),
        vec2(-0.11750484798082572, 0.1268997253162244),
        vec2(0.4249501577365884, -0.3510844866084426),
        vec2(-0.43578090240216727, -0.12889371382781623),
        vec2(0.07586719938445674, 0.3731150628855976),
        vec2(-0.3197436297238845, 0.43049516646016384),
        vec2(0.18642756070396505, -0.06874105769335287),
        vec2(-0.1888918962307623, -0.44509281923105454),
        vec2(0.3086305612034951, 0.05419428038956031),
        vec2(-0.3117787331141107, 0.23123407093109194),
        vec2(0.19021486592645764, -0.28258149109763075),

        vec2(-0.15809740236546804, -0.22002085815928946),
        vec2(0.34311654153186455, 0.28073262534571863),
        vec2(-0.055887816801843404, 0.46497030351619995),
        vec2(0.47172529509549843, -0.0329591914670071),
        vec2(-0.4702819835134301, -0.41130995665641806),
        vec2(0.029054026419489176, 0.09264033463652688),
        vec2(-0.24895948160751946, 0.0012394400159095875),
        vec2(0.2504900612143476, -0.48072733079290914),
        vec2(-0.34389523071133477, -0.0957268877603979),
        vec2(0.1539052104518367, 0.40457937885312),
        vec2(-0.4034900596151533, 0.3241683467196622),
        vec2(0.09437732355411821, -0.1763629223755282),
        vec2(-0.0836884067861996, -0.3433563848828414),
        vec2(0.406106644941877, 0.15966373412494317),
        vec2(-0.35946050000052227, 0.03209545229743016),
        vec2(0.14001959735680558, -0.46828723214448686)
    );

#endif

#endif
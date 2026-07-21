#include <stdint.h>
#include "PHY/sse_intrin.h"
void nrLDPC_bnProc_BG2_R13_AVX512(int8_t* bnProcBuf,int8_t* bnProcBufRes,  int8_t* llrRes, uint16_t Z  ) {
        uint32_t M, i; 
// Process group with 2 CNs 
 M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[108 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[108 + i ], ((simde__m512i*) bnProcBuf)[108 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[114 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[108 + i ], ((simde__m512i*) bnProcBuf)[114 + i]);
}
// Process group with 3 CNs 
// Process group with 4 CNs 
       M = (2*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[120 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[114 + i ], ((simde__m512i*) bnProcBuf)[120 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[132 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[114 + i ], ((simde__m512i*) bnProcBuf)[132 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[144 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[114 + i ], ((simde__m512i*) bnProcBuf)[144 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[156 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[114 + i ], ((simde__m512i*) bnProcBuf)[156 + i]);
}
// Process group with 5 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[168 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[126 + i ], ((simde__m512i*) bnProcBuf)[168 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[174 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[126 + i ], ((simde__m512i*) bnProcBuf)[174 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[180 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[126 + i ], ((simde__m512i*) bnProcBuf)[180 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[186 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[126 + i ], ((simde__m512i*) bnProcBuf)[186 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[192 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[126 + i ], ((simde__m512i*) bnProcBuf)[192 + i]);
}
// Process group with 6 CNs 
       M = (5*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[198 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[198 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[228 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[228 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[258 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[258 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[288 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[288 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[318 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[318 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[348 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[132 + i ], ((simde__m512i*) bnProcBuf)[348 + i]);
}
// Process group with 7 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[378 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[378 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[384 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[384 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[390 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[390 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[396 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[396 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[402 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[402 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[408 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[408 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[414 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[162 + i ], ((simde__m512i*) bnProcBuf)[414 + i]);
}
// Process group with 8 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[420 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[420 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[426 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[426 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[432 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[432 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[438 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[438 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[444 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[444 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[450 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[450 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[456 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[456 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[462 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[168 + i ], ((simde__m512i*) bnProcBuf)[462 + i]);
}
// Process group with 9 CNs 
// Process group with 10 CNs 
// Process group with 11 CNs 
// Process group with 12 CNs 
// Process group with 13 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[468 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[468 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[474 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[474 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[480 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[480 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[486 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[486 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[492 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[492 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[498 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[498 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[504 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[504 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[510 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[510 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[516 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[516 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[522 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[522 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[528 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[528 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[534 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[534 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[540 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[174 + i ], ((simde__m512i*) bnProcBuf)[540 + i]);
}
// Process group with 14 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[546 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[546 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[552 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[552 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[558 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[558 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[564 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[564 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[570 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[570 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[576 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[576 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[582 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[582 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[588 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[588 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[594 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[594 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[600 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[600 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[606 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[606 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[612 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[612 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[618 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[618 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[624 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[180 + i ], ((simde__m512i*) bnProcBuf)[624 + i]);
}
// Process group with 15 CNs 
// Process group with 16 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[630 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[630 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[636 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[636 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[642 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[642 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[648 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[648 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[654 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[654 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[660 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[660 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[666 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[666 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[672 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[672 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[678 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[678 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[684 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[684 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[690 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[690 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[696 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[696 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[702 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[702 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[708 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[708 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[714 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[714 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[720 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[186 + i ], ((simde__m512i*) bnProcBuf)[720 + i]);
}
// Process group with 17 CNs 
// Process group with 18 CNs 
// Process group with 19 CNs 
// Process group with 20 CNs 
// Process group with 21 CNs 
// Process group with 22 CNs 
// Process group with <23 CNs 
// Process group with 24 CNs 
// Process group with 25 CNs 
// Process group with 26 CNs 
// Process group with 27 CNs 
// Process group with 28 CNs 
// Process group with 29 CNs 
// Process group with 30 CNs 
}

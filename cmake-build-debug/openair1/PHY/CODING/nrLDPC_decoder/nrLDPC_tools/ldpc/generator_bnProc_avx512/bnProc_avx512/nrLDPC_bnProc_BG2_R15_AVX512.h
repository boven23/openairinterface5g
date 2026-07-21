#include <stdint.h>
#include "PHY/sse_intrin.h"
void nrLDPC_bnProc_BG2_R15_AVX512(int8_t* bnProcBuf,int8_t* bnProcBufRes,  int8_t* llrRes, uint16_t Z  ) {
        uint32_t M, i; 
// Process group with 2 CNs 
// Process group with 3 CNs 
// Process group with 4 CNs 
// Process group with 5 CNs 
       M = (2*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[228 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[228 + i ], ((simde__m512i*) bnProcBuf)[228 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[240 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[228 + i ], ((simde__m512i*) bnProcBuf)[240 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[252 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[228 + i ], ((simde__m512i*) bnProcBuf)[252 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[264 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[228 + i ], ((simde__m512i*) bnProcBuf)[264 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[276 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[228 + i ], ((simde__m512i*) bnProcBuf)[276 + i]);
}
// Process group with 6 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[288 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[288 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[294 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[294 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[300 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[300 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[306 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[306 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[312 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[312 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[318 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[240 + i ], ((simde__m512i*) bnProcBuf)[318 + i]);
}
// Process group with 7 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[324 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[324 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[330 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[330 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[336 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[336 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[342 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[342 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[348 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[348 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[354 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[354 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[360 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[246 + i ], ((simde__m512i*) bnProcBuf)[360 + i]);
}
// Process group with 8 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[366 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[366 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[372 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[372 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[378 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[378 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[384 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[384 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[390 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[390 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[396 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[396 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[402 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[402 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[408 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[252 + i ], ((simde__m512i*) bnProcBuf)[408 + i]);
}
// Process group with 9 CNs 
       M = (2*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[414 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[414 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[426 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[426 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[438 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[438 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[450 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[450 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[462 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[462 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[474 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[474 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[486 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[486 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[498 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[498 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[510 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[258 + i ], ((simde__m512i*) bnProcBuf)[510 + i]);
}
// Process group with 10 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[522 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[522 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[528 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[528 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[534 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[534 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[540 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[540 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[546 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[546 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[552 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[552 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[558 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[558 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[564 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[564 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[570 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[570 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[576 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[270 + i ], ((simde__m512i*) bnProcBuf)[576 + i]);
}
// Process group with 11 CNs 
// Process group with 12 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[582 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[582 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[588 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[588 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[594 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[594 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[600 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[600 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[606 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[606 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[612 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[612 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[618 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[618 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[624 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[624 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[630 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[630 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[636 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[636 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[642 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[642 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[648 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[276 + i ], ((simde__m512i*) bnProcBuf)[648 + i]);
}
// Process group with 13 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[654 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[654 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[660 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[660 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[666 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[666 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[672 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[672 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[678 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[678 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[684 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[684 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[690 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[690 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[696 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[696 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[702 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[702 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[708 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[708 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[714 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[714 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[720 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[720 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[726 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[282 + i ], ((simde__m512i*) bnProcBuf)[726 + i]);
}
// Process group with 14 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[732 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[732 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[738 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[738 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[744 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[744 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[750 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[750 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[756 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[756 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[762 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[762 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[768 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[768 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[774 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[774 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[780 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[780 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[786 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[786 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[792 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[792 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[798 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[798 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[804 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[804 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[810 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[288 + i ], ((simde__m512i*) bnProcBuf)[810 + i]);
}
// Process group with 15 CNs 
// Process group with 16 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[816 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[816 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[822 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[822 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[828 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[828 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[834 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[834 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[840 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[840 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[846 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[846 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[852 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[852 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[858 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[858 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[864 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[864 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[870 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[870 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[876 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[876 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[882 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[882 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[888 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[888 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[894 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[894 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[900 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[900 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[906 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[294 + i ], ((simde__m512i*) bnProcBuf)[906 + i]);
}
// Process group with 17 CNs 
// Process group with 18 CNs 
// Process group with 19 CNs 
// Process group with 20 CNs 
// Process group with 21 CNs 
// Process group with 22 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[912 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[912 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[918 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[918 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[924 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[924 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[930 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[930 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[936 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[936 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[942 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[942 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[948 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[948 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[954 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[954 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[960 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[960 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[966 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[966 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[972 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[972 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[978 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[978 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[984 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[984 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[990 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[990 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[996 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[996 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1002 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1002 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1008 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1008 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1014 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1014 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1020 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1020 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1026 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1026 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1032 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1032 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1038 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[300 + i ], ((simde__m512i*) bnProcBuf)[1038 + i]);
}
// Process group with <23 CNs 
       M = (1*Z + 63)>>6;
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1044 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1044 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1050 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1050 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1056 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1056 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1062 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1062 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1068 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1068 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1074 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1074 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1080 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1080 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1086 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1086 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1092 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1092 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1098 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1098 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1104 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1104 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1110 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1110 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1116 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1116 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1122 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1122 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1128 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1128 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1134 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1134 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1140 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1140 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1146 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1146 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1152 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1152 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1158 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1158 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1164 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1164 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1170 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1170 + i]);
}
            for (i=0;i<M;i++) {
            ((simde__m512i*)bnProcBufRes)[1176 + i ] = simde_mm512_subs_epi8(((simde__m512i*)llrRes)[306 + i ], ((simde__m512i*) bnProcBuf)[1176 + i]);
}
// Process group with 24 CNs 
// Process group with 25 CNs 
// Process group with 26 CNs 
// Process group with 27 CNs 
// Process group with 28 CNs 
// Process group with 29 CNs 
// Process group with 30 CNs 
}

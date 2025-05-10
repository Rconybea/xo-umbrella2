/* @file machdep.hpp */

#pragma once

/** Carveout for LSP (language server process):
    LSP uses clang, but with the same compiler flags as primary build.
    This triggers a handful of false alarms,  in which clang complains about
    gcc builtins.

    Replace these with something innocuous.  Ok since LSP stops
    once parsing completes and does not generate code
 **/
#if __clang__ && __GNUG__

extern "C" {
    /* never defined!  must not ever generate code that relies on these */
    unsigned int fake_rdtsc();
    unsigned int fake_mm_getcsr();
    unsigned int fake_mm_setcsr();
}

/* __rdtsc: clang encounters this from <immintrin.h>, for example */
#define __rdtsc() fake_rdtsc()
#define _mm_getcsr(a) fake_mm_getcsr()
#define _mm_setcsr(a) fake_mm_setcsr()

#endif

/* end machdep.hpp */

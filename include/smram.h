#ifndef __SMRAM_H
#define __SMRAM_H

#include "../ich2/smram-ich2.h"

extern int smram_locked();
extern smram_state_t smram_save_state();
extern void smram_restore_state(smram_state_t state);
extern int smram_aseg_set_state(int open);
extern int smram_tseg_set_state(int open);

#define SMRAM_ASEG_CLOSED 0	/* SMRAM is not readable. */
#define SMRAM_ASEG_OPEN 1	/* SMRAM is readable by everybody. */
#define SMRAM_ASEG_SMMCODE 2	/* SMRAM is readable as SMM code only. */
#define SMRAM_ASEG_SMMONLY 3	/* SMRAM is readable as SMM code and data only. */

#define SMRAM_TSEG_OPEN 0

#endif

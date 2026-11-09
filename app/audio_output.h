/* SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception */
#ifndef FM1_AUDIO_OUTPUT_H
#define FM1_AUDIO_OUTPUT_H
#include "fm1.h"
/* Reset before start or while the audio ISR is excluded. */
void fm1_output_reset(void);
/* Finish one synthesized block: master gain, USB, Bluetooth, USB recording.
 * Audio ISR only; output holds FM1_BLOCK_WORDS Q23 samples. */
FM1_AUDIO_CODE void fm1_output_mix(int32_t *output);
/* Oscilloscope tap: the instrument's mono output before master gain and
 * external audio, one sample in FM1_SCOPE_DECIMATION frames (16-bit).
 * Control task only. Capture runs only while enabled. read copies the
 * newest count (<= FM1_SCOPE_SAMPLES) samples, oldest first. */
enum { FM1_SCOPE_SAMPLES = 512, FM1_SCOPE_DECIMATION = 4 };
void fm1_scope_enable(unsigned enabled);
void fm1_scope_read(int16_t *samples, unsigned count);
#endif

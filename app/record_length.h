/* SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception */
#ifndef FM1_RECORD_LENGTH_H
#define FM1_RECORD_LENGTH_H
#include <stddef.h>
#include <stdint.h>
/* Live recording keeps held-note length (issue #15). The engine records a
 * note-on with the default 50% gate and ignores its note-off; this layer
 * notes both ends at the engine's own record position and, after release,
 * extends the recorded cell over the held steps with ties.
 *
 * record_length_event: one channel message that was posted to the engine.
 * source 0 = control task (panel keys), 1 = MIDI routing task. Each source
 * has its own single-producer queue; no blocking or engine edits here.
 * record_length_poll: control task only; applies completed notes. */
void record_length_event(unsigned source, const uint8_t *message, size_t bytes);
void record_length_poll(void);
#endif

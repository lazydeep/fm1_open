/* SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception */
#include "record_length.h"
#include "instrument.h"
#include <string.h>
#ifdef FM1_HOST_TEST
#include <stdatomic.h>
#define WORD _Atomic(unsigned)
#else
#define WORD volatile unsigned
#endif

static void barrier(void)
{
#ifndef FM1_HOST_TEST
    __asm__ volatile("csync" ::: "memory");
#endif
}

/* Single-producer queues: panel keys (control task) and MIDI routing task. */
enum { QUEUE = 64, HELD = 8, NONE = 255 };
struct event { uint32_t ms; uint8_t track, note, on, step; };
static struct { WORD read, write; struct event data[QUEUE]; } queues[2];
/* Notes held while recording: onset record position and time. */
struct held { uint32_t ms; uint8_t note, step; };
static struct held held[GROOVE_TRACKS][HELD];
static unsigned held_valid;

void record_length_event(unsigned source, const uint8_t *m, size_t n)
{
    if (source > 1 || !m || n != 3 || !groove_edit_recording()) return;
    unsigned type = m[0] & 0xf0U, channel = m[0] & 15U;
    if ((type != 0x90U && type != 0x80U) || m[1] > 127 || m[2] > 127) return;
    unsigned on = type == 0x90U && m[2];
    for (unsigned t = 0; t < GROOVE_TRACKS; ++t) {
        /* The engine's routing: every non-OFF track listening on the channel;
         * only NORMAL tracks record. */
        if (groove_track_role(t) != GROOVE_TRACK_NORMAL || groove_midi_input_channel(t) != channel) continue;
        unsigned w = queues[source].write;
        if (w - queues[source].read >= QUEUE) return; /* full: that note keeps 50% */
        queues[source].data[w % QUEUE] = (struct event){fm1_millis(), (uint8_t)t, m[1], (uint8_t)on,
                                                        (uint8_t)groove_sequence_record_position(t)};
        barrier();
        queues[source].write = w + 1U;
    }
}

static int same_notes(const struct groove_step *a, const struct groove_step *b)
{
    return a->velocity && b->velocity && a->note == b->note && (a->drums & 31U) == (b->drums & 31U);
}
/* Give the cell recorded at `start` the played length, in 1/16 steps: whole
 * steps tie into copies of the cell, the last step takes the remainder as
 * its gate (0 = full). Stops before a different note inside the span. */
static void extend(unsigned t, unsigned start, unsigned units)
{
    unsigned length = groove_sequence_length(t);
    if (groove_pad_count(t) || !length || start >= length) return;
    struct groove_step base;
    groove_sequence_read(t, start, &base, 1);
    if (!base.velocity) return; /* not recorded (e.g. chord table full) */
    if (!units) units = 1;
    unsigned steps = (units + 15U) / 16U, last = units % 16U;
    if (steps > length) { steps = length; last = 0; }
    for (unsigned k = 1; k < steps; ++k) {
        struct groove_step cell;
        groove_sequence_read(t, (start + k) % length, &cell, 1);
        if (cell.velocity && !same_notes(&cell, &base)) { steps = k; last = 0; break; }
    }
    for (unsigned k = 0; k < steps; ++k) {
        struct groove_step cell = base;
        unsigned gate = k + 1U < steps || !last ? 15U : last; /* 15 = full step */
        cell.drums = (uint16_t)((cell.drums & ~((15U << 5) | (1U << 12))) | (gate << 5));
        if (k + 1U < steps) cell.drums |= 1U << 12; /* tie into the next step */
        if (k) cell.drums &= (uint16_t)~(7U << 9); /* continuations always play */
        groove_sequence_replace(t, (start + k) % length, &cell, 1);
    }
}

void record_length_poll(void)
{
    if (!groove_edit_recording()) {
        /* Releases after REC stops are not queued; forget stale notes. */
        if (held_valid) { memset(held, NONE, sizeof held); held_valid = 0; }
        for (unsigned s = 0; s < 2; ++s) queues[s].read = queues[s].write;
        return;
    }
    if (!held_valid) { memset(held, NONE, sizeof held); held_valid = 1; }
    unsigned pending = 0;
    for (unsigned s = 0; s < 2; ++s) pending |= queues[s].read != queues[s].write;
    if (!pending) return;
    /* Let the engine store the note-ons first, then edit their cells. */
    groove_edit_poll();
    for (unsigned s = 0; s < 2; ++s)
        while (queues[s].read != queues[s].write) {
            unsigned r = queues[s].read;
            barrier();
            struct event e = queues[s].data[r % QUEUE];
            barrier();
            queues[s].read = r + 1U;
            struct held *h = held[e.track], *slot = NULL;
            for (unsigned k = 0; k < HELD; ++k)
                if (h[k].note == e.note) slot = &h[k];
            if (e.on) {
                for (unsigned k = 0; k < HELD && !slot; ++k)
                    if (h[k].note == NONE) slot = &h[k];
                if (slot) *slot = (struct held){e.ms, e.step < 254 ? e.note : NONE, e.step};
                continue;
            }
            if (!slot) continue;
            struct held note = *slot;
            slot->note = NONE;
            if (e.step >= 254) continue;
            /* A chord or overlapping notes from the same step end together. */
            unsigned shared = 0;
            for (unsigned k = 0; k < HELD; ++k) shared |= h[k].note != NONE && h[k].step == note.step;
            if (shared) continue;
            /* Played time at the current tempo; without one (MIDI clock not
             * locked yet) fall back to whole steps between record positions. */
            unsigned bpm = groove_tempo_current(), length = groove_sequence_length(e.track), units;
            if (bpm)
                units = (unsigned)(((uint64_t)(e.ms - note.ms) * bpm * 64U + 30000U) / 60000U);
            else if (length && note.step < length && e.step < length)
                units = 16U * ((e.step + length - note.step) % length);
            else continue;
            extend(e.track, note.step, units);
        }
}

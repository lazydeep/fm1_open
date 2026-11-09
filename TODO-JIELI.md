# Da fare con il toolchain JieLi 2.5.3

Verifiche rimandate perché in questo ambiente il firmware non si può compilare
(serve Windows + JieLi 2.5.3, vedi README). Riguardano le modifiche del branch
`ccr-660ce868-oe7otd`.

## 1. Build del firmware

- [ ] `FM1_PI32_TOOLCHAIN` punta a `C:\JL\pi32`; lo SHA-256 di `pi32/bin/clang.exe`
      è quello del README.
- [ ] Il build termina senza errori (il build usa `-flto`; i warning di `screen.c`
      con `-Oz` vanno letti).
- [ ] Controllare nel map file l'occupazione di RAM e Flash, prima e dopo:
  - `scope_ring` (1 KB, `app/audio_output.c`);
  - `samples` statico in `scope_capture` (1 KB) e `scope_trace` (224 B) in `app/screen.c`;
  - il codice in più nella sezione `.volatile_ram_code` (`fm1_output_mix` è `FM1_AUDIO_CODE`);
  - code e note tenute di `app/record_length.c` (circa 1,6 KB).
- [ ] Nessun errore `-Wframe-larger-than=2560` (`scope_capture` tiene 224 B sullo stack).

## 2. INIT DX7 VOICE (EDIT+SAVE e menu ACTIONS)

- [ ] EDIT+SAVE, in entrambi gli ordini, su una traccia DX7: carica INIT VOICE
      e apre QUICK EDIT; tenendo premuto non si apre l'EDIT avanzato.
- [ ] SAVE premuto per primo: la pagina di salvataggio progetto si chiude
      senza salvare.
- [ ] EDIT+SAVE su traccia non DX7: avviso "INIT VOICE: DX7 TRACKS ONLY",
      nessuna modifica.
- [ ] HOME poi EDIT apre ancora QUICK EDIT senza cancellare nulla.
- [ ] SEQ+EDIT → INIT DX7 VOICE → SAVE → compare "REPLACE TRACK TIMBRE?" → SAVE.
- [ ] Il suono è una sinusoide pura (OP1), nome "INIT VOICE", tutti gli operatori attivi.
- [ ] HOME+SAVE (UNDO) ripristina la voce precedente.
- [ ] Su una traccia non DX7: "UNAVAILABLE / INCOMPATIBLE", nessuna modifica.
- [ ] Su una traccia PLAY: funziona, e non sporca il progetto (comportamento di `groove_fm_import`).
- [ ] DX7 PATCH NAME, BLUETOOTH / CLOCK e STOP ALL funzionano ancora dalla nuova posizione nel menu.

## 3. Oscilloscopio (DX7 QUICK EDIT, EDIT ed ENV degli altri motori)

- [ ] Il tracciato compare e resta stabile con una nota tenuta (prova su note
      basse, medie, alte).
- [ ] Reattività: girando K1–K4 e PRESETS mentre suonano accordi a 8 voci, i
      valori si aggiornano senza ritardi visibili.
- [ ] Nessun messaggio "AUDIO OVERLOAD" o "CPU GUARD - VOICES ADAPTED" in più
      rispetto al firmware senza oscilloscopio, a parità di patch e polifonia.
- [ ] Con audio USB o Bluetooth in ingresso, l'oscilloscopio mostra solo lo
      strumento (previsto).
- [ ] Pagine EDIT ed ENV di 303, PSG, Pluck, 808 e Sampler: parametri e
      pad compattati leggibili, oscilloscopio sotto (y 132–177), nessuna
      sovrapposizione con la riga d'aiuto a y 181.
- [ ] Uscendo dalla pagina la cattura si ferma (`fm1_scope_enable(0)`).
- [ ] Se l'interfaccia rallenta: aggiornare ogni 66 ms invece di 33, o ridurre
      l'altezza dell'area (`scope_capture`/`scope_draw`, parametro `height`).

## 4. Durata delle note in registrazione dal vivo (issue #15)

- [ ] Traccia NORMAL, PLAY + REC: una nota tenuta per più step si riascolta
      lunga, senza ribattuta (step successivi con TIE, gate pieno sull'ultimo).
- [ ] Note brevi: il gate dello step corrisponde alla durata (SEQ 2/8 STEP
      DETAILS → GATE).
- [ ] Accordi tenuti e note in legato (la nota precedente si ferma dove inizia
      la successiva).
- [ ] Note da MIDI esterno (DIN, USB, BLE), anche con clock MIDI esterno.
- [ ] Con LOCAL OFF i tasti del pannello non registrano (come prima).
- [ ] Batteria (808, Sampler) e registrazione a step (da fermo) invariate.
- [ ] HOME+SAVE (UNDO) dopo una registrazione annulla anche le durate.
- [ ] `fm1_millis()` e `groove_sequence_record_position()` funzionano dal task
      di routing MIDI (`fm1_app_midi_received`).

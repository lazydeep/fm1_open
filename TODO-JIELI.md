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
  - il codice in più nella sezione `.volatile_ram_code` (`fm1_output_mix` è `FM1_AUDIO_CODE`).
- [ ] Nessun errore `-Wframe-larger-than=2560` (`scope_capture` tiene 224 B sullo stack).

## 2. INIT DX7 VOICE (menu ACTIONS)

- [ ] SEQ+EDIT → INIT DX7 VOICE → SAVE → compare "REPLACE TRACK TIMBRE?" → SAVE.
- [ ] Il suono è una sinusoide pura (OP1), nome "INIT VOICE", tutti gli operatori attivi.
- [ ] HOME+SAVE (UNDO) ripristina la voce precedente.
- [ ] Su una traccia non DX7: "UNAVAILABLE / INCOMPATIBLE", nessuna modifica.
- [ ] Su una traccia PLAY: funziona, e non sporca il progetto (comportamento di `groove_fm_import`).
- [ ] DX7 PATCH NAME, BLUETOOTH / CLOCK e STOP ALL funzionano ancora dalla nuova posizione nel menu.

## 3. Oscilloscopio (DX7 QUICK EDIT)

- [ ] Il tracciato compare e resta stabile con una nota tenuta (prova su note
      basse, medie, alte).
- [ ] Reattività: girando K1–K4 e PRESETS mentre suonano accordi a 8 voci, i
      valori si aggiornano senza ritardi visibili.
- [ ] Nessun messaggio "AUDIO OVERLOAD" o "CPU GUARD - VOICES ADAPTED" in più
      rispetto al firmware senza oscilloscopio, a parità di patch e polifonia.
- [ ] Con audio USB o Bluetooth in ingresso, l'oscilloscopio mostra solo lo
      strumento (previsto).
- [ ] Uscendo dalla pagina la cattura si ferma (`fm1_scope_enable(0)`).
- [ ] Se l'interfaccia rallenta: aggiornare ogni 66 ms invece di 33, o ridurre
      l'altezza dell'area (`SCOPE_H`).

## 4. Analisi di spettro (quando sarà implementata)

- [ ] Misurare il tempo della FFT nel task di controllo (`fm1_millis()` prima e
      dopo, o un timer più fine) e verificare la reattività come al punto 3.
- [ ] Controllare la RAM aggiuntiva (buffer di cattura a piena frequenza e buffer FFT).

# What's New in VUURWERK v1.5.0

**The short version:** the spectrum analyzer learned to hunt voices and hold
on to them, a new rescue key un-jams a silent VFO in one gesture, and the two
long-standing spectrum-exit gremlins -- the surprise VFO flip and the
deaf-after-exit radio -- are fixed for good. If you are upgrading from any
v1.3.x build, this page is your tour.

Flashing is the same as always (see [FLASHING.md](FLASHING.md)). Your
channels and settings survive the upgrade.

---

## 1. The VOX Voice Lock -- hunt, seek, lock, carry home

The Voice-Seeking Spectrum (F+5, then STAR until the corner says `VOX`) now
gives you the complete find-a-conversation workflow:

```
        THE HUNT                        YOUR KEYS
+--------------------------------+
| -130/-50              [BATT##] |   UP    seek next voice above
| 146.552.50        FM     VOX   |   DOWN  seek next voice below
|  .     .    .      .    .      |   PTT   lock onto this voice
|        |                #      |   EXIT  leave (locked freq
|        |       ,        #      |         follows you out)
|  _.____|_____,:,________#____  |
| ...:....|....:....^....|....:  |   full bar = strong voice
| 145.760       >80.0k  148.960  |   half bar = probable voice
+--------------------------------+   dots on top = places UP/DOWN can land
```

**Hunt.** In VOX mode only bins that score as human voice draw bars; carriers
and birdies stay flat on the floor. The sweep auto-stops and listens when it
finds voice.

**Seek.** UP hops to the next voice above where you are, DOWN below. If there
is no voice that way, the key simply steps the band like normal navigation --
UP and DOWN always move you. Hold a key and the radio patrols until it lands
on a voice (you'll hear a short ascending two-tone chord when it does).

**Lock.** Found a conversation worth staying on? Tap PTT.

```
        +--------------+
        |    LOCKED    |    <- flashes over the spectrum, then fades
        +--------------+
```

The radio parks on that frequency with the squelch open -- you hear both
sides of the QSO and anyone else who keys up, and it holds through any pause,
however long. UP or DOWN releases the lock and resumes seeking.

**Carry home.** Press EXIT while locked (with the radio in frequency mode)
and the locked frequency becomes your VFO frequency:

```
   spectrum, LOCKED on 146.552.50
              |
            EXIT
              v
+--------------------------------+
| A  146.552.50       FM   H  N  |   <- already tuned; press PTT to answer
+--------------------------------+
```

The write is fenced hard: frequency mode only, same band only, and a memory
channel can never be modified by it. In memory mode a locked exit simply
returns you to your channel unchanged.

---

## 2. F+EXIT -- the reset-to-simplex rescue key

The most common "my radio is broken" report is not a broken radio. It is one
VFO carrying a leftover setting -- an RX tone that never matches (green light,
no audio), a repeater offset, reverse, or a scrambler -- while the other VFO
hears fine.

One gesture now clears all of it. Press F, then EXIT:

```
   before                              after
+---------------------+           +---------------------+
| A 146.520.00  R CTC |   F+EXIT  | A 146.520.00   FM   |
|   +0.600  rev  scrm |  ------>  |   RX = TX, no tones |
| (green light, mute) |  SIMPLEX  | (hears everything)  |
+---------------------+           +---------------------+
```

What it clears on the active VFO: RX and TX CTCSS/DCS tones, repeater offset
and direction (TX = RX, true simplex), frequency reverse, scrambler, and the
modulation is set back to FM. The change is saved, so it survives a power
cycle. It works in frequency mode only -- on a memory channel you get a
`FREQ ONLY` toast and nothing is touched.

If a radio ever seems deaf on one side: F+EXIT, then listen again.

---

## 3. Stability: the spectrum exit is now boring (in the best way)

Two long-hunted field reports are fixed:

**"Exiting the spectrum flips me to the other VFO."** Gone. Every exit now
lands you on the VFO you entered from, every time -- including the sneaky
variant where holding the EXIT button a beat too long re-triggered a VFO swap
on the main screen after you had already left.

**"After the spectrum, the radio looks fine but hears nothing."** Gone. The
exit path now always retunes the radio to your displayed frequency and fully
restores the audio path, with dual watch given a fresh, orderly restart. No
more radios that need a power cycle after a spectrum session.

```
   spectrum ---- EXIT ----> your VFO, your frequency, audio alive
                            (lock or no lock, dual watch or not)
```

v1.5.0 is a stabilization release: every one of these paths was exercised
end-to-end and re-verified before shipping.

---

## 4. Spectrum modes: NORM / PEAK / VOX (MTI removed)

The STAR key now cycles three display modes instead of four:

```
      STAR              STAR              STAR
 NORM ----->  PEAK  ----->  VOX  -----> (back to NORM)

 NORM  live bars          |.|,||.:|,.|      what's on the air right now
 PEAK  bars + peak dots   '''|''.''|''      catches brief transmissions
 VOX   voice bars only    _.|____#___       shows conversations, not carriers
```

The MTI (Moving Target Indicator) mode is retired -- in practice nobody used
it, and removing it paid for the About screen's feature list and the lock
work. PEAK, which people genuinely rely on, stays exactly as it was. The
current mode is always shown in the top-right corner of the spectrum, and a
brief overlay confirms each STAR press.

---

## 5. Also in the v1.3.3 -> v1.5.0 arc

If you are coming from v1.3.2, you also pick up:

- **Adaptive scan**: empty channels are pre-checked in ~30 ms and skipped, so
  scanning quiet bands runs about 2-3x faster with weak-signal sensitivity
  unchanged. Tone-coded memory channels now get a long-enough dwell that the
  tone decoder actually confirms them (they used to be walked past).
- **The radio keeps its hands off the dial while yours are on it**: dual watch
  and VFO Split pause their automatic retuning while you are holding a key,
  and resume the moment you let go.
- **Punchier TX audio**: the middle and upper mic-gain steps drive about
  1.5-2 dB harder.
- **A memory-safety sweep**: a display-renderer overrun that could silently
  corrupt saved settings ("my settings changed on their own") was found and
  fixed, along with a series of smaller hardening fixes.
- **DCS tail elimination polarity fix**: tail-free endings now work correctly
  on DCS channels too.

The full engineering record lives in [CHANGELOG.md](CHANGELOG.md).

---

## Cheat sheet delta (what changed on the keys)

| Gesture | Before | Now |
|---|---|---|
| STAR (in spectrum) | NORM / PEAK / MTI / VOX | NORM / PEAK / VOX |
| UP / DOWN (in VOX) | wrap-around hop, dead-end beep | directional seek, falls through to band navigation |
| PTT (in VOX) | register editor screen | lock the voice (`LOCKED` banner) |
| EXIT (locked, freq mode) | frequency lost | frequency carries to your VFO |
| F+EXIT (main screen) | nothing | reset VFO to simplex (`SIMPLEX` toast) |

Everything else is where you left it.

---

*VUURWERK v1.5.0. Standard: KC3TFZ.*

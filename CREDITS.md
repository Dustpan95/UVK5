# Acknowledgments

VUURWERK stands on the shoulders of the open UV-K5 community.

**Upstream firmware:**
- **[DualTachyon](https://github.com/DualTachyon)** — author of the original
  open UV-K5 firmware, the reverse-engineering feat that made every UV-K5
  custom firmware possible. Apache License 2.0.
- **[OneOfEleven](https://github.com/OneOfEleven)** — extensive custom
  modifications and improvements, including AM_fix, that flowed into the
  firmware VUURWERK builds on. Apache License 2.0.
- **[egzumer](https://github.com/egzumer/uv-k5-firmware-custom)** — the
  direct parent of VUURWERK: the merged, polished firmware that combined
  OneOfEleven's work with fagci's spectrum analyzer. Apache License 2.0.
- **[fagci](https://github.com/fagci)** — the spectrum analyzer heritage;
  VUURWERK's spectrum and bandscope work grew from his foundation.
  Apache License 2.0.
- **kamilsss655 (NUNU)** — bench-validated the quiet-backlight-PWM technique
  in 2024 that VUURWERK's Quiet Backlight PWM re-implements as a hook.

**CHIRP driver:**
- **Jacek Lipkowski, SQ5BPF** — author of the original Quansheng UV-K5
  CHIRP driver (GPL v2+) on which VUURWERK's driver is based, itself built
  on Dan Smith's CHIRP template.
- **mjwgeek** ([github.com/mjwgeek](https://github.com/mjwgeek)) —
  contributed the VUURWERK adaptation of the CHIRP driver, so channels and
  settings can be programmed without flashing back to stock.

**Libraries:**
- **Marco Paland** — embedded `printf` (MIT License).
- **ARM Limited** — CMSIS headers (Apache License 2.0).

**Community:**
- A special thank-you to **Scott Savage, KC7WDG**, for taking the time to
  bring real problems to my attention. His careful field reports — most
  notably the scanner going deaf — led directly to some of VUURWERK's most
  important scanning fixes. Operators like Scott make firmware better for
  everyone. 73!

---

*See [LICENSE](LICENSE) for the complete list of VUURWERK-original
contributions and all license terms.*

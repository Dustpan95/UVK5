# Measured feature costs

These isolated diagnostic builds use ARM GCC 10.3.1 and source fingerprint
`1aee4bcb432eb52b2c461ae9fed53f672734e8e784ca58922a5f6a6817db5722`.
Baseline: **61,012 / 61,440 bytes**, 428 free, 3,832 bytes static RAM.
This baseline includes the approved custom home renderer and APRS message alerts.

No feature cuts have been selected. Results are comparisons, not release profiles.

| Profile | Flash delta | Result |
|---|---:|---|
| spectrum | +7,116 bytes | Exceeds 60 KiB |
| fm_broadcast | +4,328 bytes | Exceeds 60 KiB |
| vox | +716 bytes | Exceeds 60 KiB |
| voice_prompts | +1,360 bytes | Exceeds 60 KiB |
| noaa | +1,268 bytes | Exceeds 60 KiB |
| dtmf_calling | Unmeasured | Compile error (see JSON) |
| mic_audio_bar | +304 bytes | Fits |
| bold_small_font | +628 bytes | Exceeds 60 KiB |
| 1750hz | +196 bytes | Fits |
| without_am_fix | Unmeasured | Compile error (see JSON) |
| without_adaptive_scan | -108 bytes | Fits |
| without_scan_ranges | -388 bytes | Fits |
| without_flashlight | -400 bytes | Fits |
| stub_compressor | -404 bytes | Fits |
| stub_adaptive_squelch | -1,000 bytes | Fits |
| stub_scanwatch | -384 bytes | Fits |
| stub_vfo_split | -628 bytes | Fits |
| stub_about | -324 bytes | Fits |

Module-body stub rows retain menus and hooks. Their deltas estimate only the
removed processing bodies; they are not working feature-removal configurations.
LTO means deltas are not additive. Full logs and isolated source copies remain
in ignored `build/feature-costs/`. Diagnostic linker regions are deliberately
128 KiB so oversized variants can be measured: **never flash these images**.
The production linker and verifier still enforce the real 60 KiB application
limit. No diagnostic binary is a release asset.

Two inherited compile-time configurations fail: DTMF calling references missing
upstream symbols, and disabling AM-fix leaves an unguarded menu identifier.
These failures do not affect the current baseline, and no size estimate is
assigned to them. They need repair before they are selectable options.

Reproduce with `python3 tools/measure-features.py`. Machine-readable results:
`feature-costs.json`. Any subsequent source changes require new measurements
before relying on exact byte counts for a release.

# Measured feature costs

The original whole-profile table uses source fingerprint `5f8206dcfacf2a37ada85440024afd77e62693ccbd091479105ad3f893f80611`.
ARM GCC 10.3.1, identical current radio features except the stated variant.
Baseline: **58,056 / 61,440 flash bytes**, 7,188 static RAM bytes.
No feature cuts are selected. Standalone receive-profile fits do not establish
fit with the full messaging/transmit service. Deltas are not additive.

| Variant | Flash change | Flash bytes |
|---|---:|---:|
| messaging_editor | +4,928 | 62,984 |
| font_unpacked | +564 | 58,620 |
| spectrum | +7,280 | 65,336 |
| fm_broadcast | +4,180 | 62,236 |
| vox | +776 | 58,832 |
| voice_prompts | +1,428 | 59,484 |
| noaa | +976 | 59,032 |
| dtmf_calling | Compile failure | Unmeasured |
| mic_audio_bar | +236 | 58,292 |
| bold_small_font | +500 | 58,556 |
| 1750hz | +140 | 58,196 |
| without_am_fix (repaired follow-up) | -556 | 57,500 |
| without_adaptive_scan | -108 | 57,948 |
| without_scan_ranges | -296 | 57,760 |
| without_flashlight | -532 | 57,524 |
| stub_compressor | -476 | 57,580 |
| stub_adaptive_squelch | -944 | 57,112 |
| stub_scanwatch | -364 | 57,692 |
| stub_vfo_split | -584 | 57,472 |
| stub_about | -292 | 57,764 |

Module-body stub rows measure counterfactual processing cost while retaining
menus/hooks. They are not working feature-removal configurations and must not
be offered as release builds. The compressor, squelch, split and scan features
remain enabled in the normal source and build.

The inherited optional DTMF calling configuration references missing upstream
symbols and remains unmeasured. AM-fix-off is now repaired: the shared front-end
gain table is always compiled for FM gain staging, while AM processing remains
conditional. The default AM-fix-enabled raw/packed images are byte-identical to
the prior checkpoint. The repaired optional configuration uses 57,500 bytes,
saving 556 against its same-source 58,056 baseline. This is an option measurement;
AM fix remains enabled in the normal firmware. `am-fix-repaired-costs.json`
records the follow-up fingerprint. `am-fix-cost-blocker.json` retains the earlier
failure for diagnosis, and the main JSON table retains its original fingerprint.

Diagnostic builds enlarge FLASH to 128 KiB only in ignored copies under
`build/feature-costs/`, so overflow can be measured. **Never flash these images**.
The production linker/verifier retain 61,440 bytes and 16 KiB SRAM.

Reproduce with `python3 tools/measure-features.py`; `--profile NAME` selects
a subset plus baseline. JSON records the measured source fingerprint. Later
source changes require new measurements before assigning exact release costs.
See `FLASH-FEASIBILITY.md` for the current complete-profile problem.

## Application UART and wide receive follow-up

`programming-costs.json` records a separately fingerprinted RX comparison:
application UART off uses 55,676 bytes (2,380 saved), and wide receive off uses
57,984 bytes (72 saved), against the same-source 58,056-byte baseline. Removing
application UART would remove firmware serial programming/CHIRP support; the
bootloader updater is separate. Neither option is selected. Do not subtract RX
costs from the TX total: full-transport combinations are measured independently
in `protocol-size-results.json` when `tools/measure-protocol.py --transport-options`
is run. Those remain incomplete oversized diagnostic prototypes, not releases.

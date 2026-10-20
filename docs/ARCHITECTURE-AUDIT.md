# Feature-preserving flash investigation

The complete candidate uses **67,896 / 61,440 bytes**: **6,456 bytes over**.
This is a measurement of the current implementation, not a minimum size for
the requested functionality. No further feature removals are selected.
Automatic tone search, flashlight and background VFO activity scanning remain
the only three user-selected removals.

## Evidence that changes the investigation

`tools/attribute-flash.py` builds the actual transport profile in an ignored,
enlarged-linker copy, then rebuilds with debug line information. It rejects the
result unless application bytes remain identical. The measured raw SHA-256 is
`b0b5af071e245450df9dd257eeb22e6e7943a2d141773d4c8ff83d4c66d9ede5`.
The compiler is ARM GCC 10.3.1. Full results are in
[`flash-source-attribution.json`](flash-source-attribution.json).

The disassembly attributes approximately 52,234 instruction bytes to source.
The remaining 15,662 bytes include constants, literal pools, vectors,
initialized data and alignment. Debug attribution after LTO is approximate;
**these counts are not removable feature costs**.

| Source area | Attributed instruction bytes |
|---|---:|
| Application scheduling and radio state (`app/app.c`) | 4,262 |
| Radio menu behavior (`app/menu.c`) | 3,282 |
| Radio settings and EEPROM (`settings.c`) | 2,716 |
| Radio configuration (`radio.c`) | 2,522 |
| Custom home (`ui/home.c`) | 2,072 |
| Radio menu drawing (`ui/menu.c`) | 2,032 |
| APRS message editing and frame construction | 1,728 |
| Shared formatter | 1,496 |
| APRS station/inbox session logic | 1,332 |
| APRS outgoing queue and retries | 1,058 |

The large linked `Main` symbol is an optimizer aggregation, not an independent
9 KB feature. Its instructions include scheduling, squelch, UART, startup,
gain staging, compressor and many drivers. Counting it separately from those
features would misrepresent the architecture.

The old complete home renderer is already replaced. `UI_DisplayMain` routes to
the custom home or APRS browser and retains lock/battery/error handling. There
is no unused second full home screen whose deletion would recover the deficit.

## Upstream comparison

TA1JS commit `6320cdac7a2dce46f88027d220ff67e47eb0e3fb` is the same pinned
source reviewed originally. Its `app/aprs_minimal.c:187` explicitly documents
measured **1200/1800 Hz** native FSK transmission despite the broader README's
1200/2200 wording. That smaller transmitter cannot establish compliance with
the requested standard-compatible transmit behavior. Its message composition
also uses a 30-character buffer and one retained personal message; that is not
equivalent to our approved editor, inbox and traffic-monitor design.

This does not prove our software tone transmitter or messaging architecture is
size-optimal. It does mean that comparing upstream binary size alone omits
important differences in behavior. Physical RF timing, deviation and external
TNC interoperability still require hardware validation.

## Dispatch simplification experiment

In an isolated copy, replace the five `c->ops` callback invocations in
`experiments/aprs_channel.c` with their actual `HARDWARE_*` targets and include
`aprs_hardware.h`. Keep the surrounding state machine, settings and safeguards.
The real transport build becomes **67,892 bytes**, only **4 bytes smaller**.

This probe has not been adopted or behavior-validated. It shows that simply
flattening this callback boundary is not a useful solution to the 6,456-byte
gap under the pinned compiler. It does not establish that all APRS structure
is optimal or that hardware restoration checks can be removed.

## Remaining engineering work

Investigate compact representations and shared behavior across the menu,
settings and APRS views, preserving controls, settings, accepted layouts and
radio behavior. Measure complete builds after each change and compare actual
behavior, rather than treating source length, symbol size or isolated stub
savings as proof. The earlier enum-label table experiment saved only 8 bytes;
a credible menu redesign needs more than moving labels into a table.

The complete requested build still does not fit. This audit supplies a better
basis for implementation work; it does not promise a fit, approve more cuts,
or turn the fitting RX-only bench into the requested final release.

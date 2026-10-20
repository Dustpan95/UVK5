# Feature space review for owner selection

The owner selected **G: automatic CTCSS/DCS tone search**, **C: flashlight**,
and **N: secondary-VFO activity scanning (VUURWERK VFO Split)** for removal.
All three are excluded from current builds. Ordinary A/B selection, dual watch,
channel/frequency scanning, Scan+Watch and configured CTCSS/DCS remain.

The complete transport candidate is now **67,896 / 61,440 bytes**:
**6,456 bytes remain to recover**. Removing N saved 816 bytes from the previous
68,920-byte candidate. Shared APRS setup handlers recovered another 208 bytes
without changing controls, screens or capabilities. Together, this step saved
1,024 bytes. Current static RAM is 8,684 + 2,048 reserved stack out of 16,384.
Default RX is **54,164** and development RX-session is **57,324 bytes**.
Frozen historical downloads are unchanged; current CI artifacts use current
defaults. RF/hardware testing remains unrun.

[What N actually did and what remains](VFO-SPLIT-REMOVAL.md) explains the removed
background activity scan. Its auto-switch/log-only enum names were not
implemented modes in this fork; only the activity-alert path was wired.

**The individual tables below remain the measurement snapshot at
[95f4d64](https://github.com/Dustpan95/UVK5/tree/95f4d64decbaa4f4df972833834284d4fa525855),
before N and the setup sharing change.** N is already removed; other individual
figures are reference estimates, not exact additional savings on 67,896 bytes.
New choices must be built and measured together. The earlier
[pre-removal results](transport-feature-costs.json) are also retained.

All measurements below use the same complete candidate, ARM GCC 10.3.1 and
source fingerprint in [the current full results](remaining-feature-costs.json). No
production linker capacity was changed. Diagnostic binaries must never be
flashed. All current reported probes built successfully; behavioral tests of
these removal probes are deliberately not claimed.

## Existing build switches

These are measured changes to real build switches. Savings are relative to
68,920 bytes, one switch at a time. They still need combined validation.

| ID | Candidate removal | Flash saved | What you lose |
|---|---|---:|---|
| A | Application serial programming | 2,596 B | CHIRP and application serial EEPROM/programming access. Bootloader flashing is a separate path. |
| B | AM fix | 512 B | Automatic AM reception gain correction; AM itself remains. |
| D | Scan ranges | 32 B | User-selected scan start/stop limits; ordinary scanning remains. |
| E | Extended receive coverage | 76 B | Extended outer RX bands; normal VHF/UHF coverage remains. |
| F | Adaptive scan dwell | 88 B | Adaptive dwell enhancement; ordinary scanning remains. |

`ENABLE_CUSTOM_MENU_LAYOUT=0` saved **zero bytes**. The inherited categorized
menu is not removed by that switch in the current source; a smaller settings
interface would require actual implementation and comparison.

## Radio enhancements requiring code changes

These are **estimates from removing module bodies in temporary copies**. Their
remaining buttons, settings, callers and interactions are not cleaned up into
a usable firmware. Proper removal can have a different cost. They are evidence
for choosing work, not promised exact savings or supported release variants.

| ID | Candidate removal | Probe saving | What you lose |
|---|---|---:|---|
| H | Channel/frequency scanning | 1,420 B | Automatic scanning through channels or VFO frequencies; manual tuning remains. |
| I | Adaptive squelch | 792 B | VUURWERK's dynamic squelch adjustment. This was an original requested feature. |
| J | RX gain staging | 468 B | Dynamic receiver gain adjustment for strong signals. |
| K | TX compressor | 352 B | Voice microphone compression. This was an original requested feature. |
| L | Mini bandscope | 424 B | The small RF activity/spectrum strip. |
| M | Scan+Watch | 332 B | Scanning while periodically checking a watch VFO. |
| N — removed | VFO Split enhancement | 540 B | VUURWERK's secondary-VFO scan/activity mode; this does not mean repeater offsets. |
| O | Current About screen | 284 B | Feature list and voltage-sag page. Required notices and prominent original credits must remain. |
| P | Enhanced squelch-tail handling | 304 B | Additional suppression of repeater tone/code tail noise. Configured CTCSS/DCS remains. |
| Q | Signal classifier | 264 B | Signal classification display and classification input to other enhancements. |
| R | Signal quality analysis | 200 B | Quality indicator and quality input used by other enhancements. |
| S | Activity-weighted dual-watch dwell | 216 B | Adaptive preference for the active VFO; the core dual-watch function is separate. |
| T | Side-button toast feedback | 316 B | Brief text confirming a side-button action; the actions remain. |
| U | CTCSS transmit lead-in | 104 B | Brief tone-before-voice behavior intended to open a repeater. Configured CTCSS remains. |
| V | Scan-speed telemetry | 64 B | Channels-per-second measurement/display. |
| W | Backlight fade | 96 B | Gradual fade; normal backlight operation remains. |
| X | TX voltage-sag display | 168 B | Sag measurement used on the About screen. Battery percentage/protection are separate. |

A module can affect several other features: removing gain/classification/quality
inputs can change adaptive squelch behavior. These interactions need explicit
implementation tests. Basic TX admission, battery safeguards, calibration,
PA shutdown and bootloader recovery are not proposed as expendable features.

## APRS-specific choices and display costs

These are also component-removal probes, not ready-to-flash cuts. Overlapping
rows must not be added together.

| ID | Component | Probe saving | Consequence |
|---|---|---:|---|
| Y | All APRS position decoding | 836 B | Received packets no longer yield coordinates; conflicts with the requested position display. |
| Y1 | Only compressed positions | 192 B | Stations sending base-91 compressed coordinates lose decoded positions; other formats remain. Included within Y. |
| Y2 | Only Mic-E positions | 288 B | Mic-E stations lose decoded positions; other formats remain. Included within Y. |
| Z | Static beacon encoder only | 384 B | Static beacon generation stops; messaging remains. Position/beacon setup cleanup could save additional bytes but is not measured here. |
| UI | All APRS setup drawing code | 1,884 B | Removes the setup display entirely, which is unusable without replacement. This is the current component cost, not a promised saving from a compact redesign. |

The editor, callsign entry, quick messages, position entry, status and radio
service share code. Removing one screen does not necessarily remove all of its
backend, and removing a common helper can affect more than one feature.

## Where the rest goes

The fitting development RX-session profile uses **57,324 bytes**; the complete
candidate adds **10,572 bytes** for the full messaging/setup/transmit paths.
This comparison includes shared-code changes and is not the cost of TX alone.
The minimal RX profile is **54,164 bytes**. These are different feature profiles,
not interchangeable claims of a finished transmit release.

For scale, the complete candidate's linked symbols include the following.
Inlining means these are compilation units of work, not clean feature budgets:

| Linked code/data | Bytes | Includes / qualification |
|---|---:|---|
| Main | 9,144 | Startup/main-loop work and inlined foreground handlers; not just the loop itself. |
| Main-screen rendering | 3,724 | Custom home, APRS browser/editor/setup drawing inlined into the display path. |
| Main radio key processing | 2,864 | Ordinary radio keypad operation and mode handling. |
| Settings display | 2,608 | Menu/value drawing; settings input/saving use additional code. |
| Packet decode | 2,554 | Packet decode/routing and inlined helpers. |
| Settings key handling | 1,596 | Navigating/editing settings; additional acceptance code is 1,532 B. |
| Packed bitmap fonts | 1,765 | Shared radio/APRS glyphs, already compressed losslessly. |
| Packed menu hints | 644 | Existing explanatory menu text, already compressed losslessly. |

These symbol sizes come from the actual baseline ELF; they are not removal
measurements. Other radio drivers, calibration/settings handling, tables,
interrupts, protocol/tone scheduling, UI helpers and constants occupy the rest.
A new menu might reduce some settings code, but removing its entire current
cost would leave no working settings interface.

FM broadcast radio, NOAA, voice prompts, VOX, alarm, AirCopy, spectrum application,
full DTMF calling and several other optional features are already disabled.
Their removal cannot fund this build. The mini bandscope above is a separate
feature from the already-disabled spectrum application.

README files, upstream Git history, licenses and documentation screenshots are
not copied into application flash. Removing attribution or repository files
would not recover this deficit. Received-history capacities mainly consume
RAM; cutting them is not a solution to the remaining flash shortage.

## Reproduction and next decision

Run `python3 tools/measure-features.py --transport` at the linked current
source to reproduce these results. The historical pre-removal report instead
uses the earlier `aaef790` source. It builds an enlarged-linker
baseline and each diagnostic variant in ignored copies, preserving original
source notices. `python3 tools/measure-transport.py` measures the unmodified
transport profile. No command here flashes hardware.

Select the IDs you can lose and any that must stay. We will implement and build
the combination before claiming that it fits. Savings are not additive under
LTO, and a fitting candidate still needs regression and physical RF validation.


## Feature-preserving size experiments

Selective shared function boundaries recovered **208 bytes**, and are adopted
only for APRS setup functions. Menu boundaries saved 200 bytes alone but grew
the image when combined with setup boundaries; that combination was rejected.
Home/message boundary variants also grew the image.
[Exact results](transport-inlining-results.json) record the 68,104-byte post-N
baseline and each variant. No screens or controls changed.

A table-driven formatter for 15 settings choices compiled to 67,888 bytes
versus the current 67,896: only **8 bytes** saved. It was not adopted; behavioral
equivalence is unverified. [The probe](menu-table-size-results.json) is retained
so this is not mistaken for a demonstrated multi-kilobyte UI saving. These
experiments do not establish a fitting full-TX build with every remaining
feature. Further major restructuring or an explicit feature/release tradeoff
is still needed; no further feature cuts are selected.

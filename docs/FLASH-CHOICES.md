# Feature space review for owner selection

The owner selected **G: automatic CTCSS/DCS tone search** and **C: flashlight**
for removal. Both are now excluded from the current build. Manual RX/TX tone
settings, channel/frequency scanning and all other unselected features remain.

The actual transport candidate is now **68,920 bytes**, against **61,440
available**: **7,480 bytes remain to recover**, plus some margin for fixes.
The combined removal saved **3,168 bytes** from the previous 72,088-byte build.
This includes removing the real tone-search screen, dispatch entries, results,
menu/key entry points and periodic work; the earlier isolated probes were not
complete removals. RAM fits: 8,712 static + 2,048 stack out of 16,384 bytes.

Default RX is now **54,680 bytes**, and development RX-session is **57,928**.
The frozen historical RX bench downloads have not been silently replaced.
Current CI artifacts use the new defaults. No radio was flashed or RF-tested.

**The individual tables below are the pre-removal snapshot at
[aaef790](https://github.com/Dustpan95/UVK5/tree/aaef79042ec7b7406c0a6f19fbeabe8784a00ce2),
for reference—not new savings available on top of 68,920 bytes. C and G are
already removed.** New selections must be measured together on current source.

All measurements below use the same complete candidate, ARM GCC 10.3.1 and
source fingerprint in [the full results](transport-feature-costs.json). No
production linker capacity was changed. Diagnostic binaries must never be
flashed. All current reported probes built successfully; behavioral tests of
these removal probes are deliberately not claimed.

## Existing build switches

These are measured changes to real build switches. Savings are relative to
72,088 bytes, one switch at a time. They still need combined validation.

| ID | Candidate removal | Flash saved | What you lose |
|---|---|---:|---|
| A | Application serial programming | 2,180 B | CHIRP and application serial EEPROM/programming access. Bootloader flashing is a separate path. |
| B | AM fix | 712 B | Automatic AM reception gain correction; AM itself remains. |
| C — removed | Flashlight | 524 B | LED flashlight controls/watchdog. |
| D | Scan ranges | 252 B | User-selected scan start/stop limits; ordinary scanning remains. |
| E | Extended receive coverage | 236 B | Extended outer RX bands; normal VHF/UHF coverage remains. |
| F | Adaptive scan dwell | 72 B | Adaptive dwell enhancement; ordinary scanning remains. |

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
| G — removed | Automatic CTCSS/DCS tone search | 2,092 B | Finding an unknown tone/code on a tuned frequency. Normal configured CTCSS/DCS remains. Ordinary scanning is separate. |
| H | Channel/frequency scanning | 1,648 B | Automatic scanning through channels or VFO frequencies; manual tuning remains. |
| I | Adaptive squelch | 968 B | VUURWERK's dynamic squelch adjustment. This was an original requested feature. |
| J | RX gain staging | 696 B | Dynamic receiver gain adjustment for strong signals. |
| K | TX compressor | 544 B | Voice microphone compression. This was an original requested feature. |
| L | Mini bandscope | 532 B | The small RF activity/spectrum strip. |
| M | Scan+Watch | 464 B | Scanning while periodically checking a watch VFO. |
| N | VFO Split enhancement | 460 B | VUURWERK's secondary-VFO scan/activity mode; this does not mean repeater offsets. |
| O | Current About screen | 444 B | Feature list and voltage-sag page. Required notices and prominent original credits must remain. |
| P | Enhanced squelch-tail handling | 416 B | Additional suppression of repeater tone/code tail noise. Configured CTCSS/DCS remains. |
| Q | Signal classifier | 376 B | Signal classification display and classification input to other enhancements. |
| R | Signal quality analysis | 360 B | Quality indicator and quality input used by other enhancements. |
| S | Activity-weighted dual-watch dwell | 344 B | Adaptive preference for the active VFO; the core dual-watch function is separate. |
| T | Side-button toast feedback | 324 B | Brief text confirming a side-button action; the actions remain. |
| U | CTCSS transmit lead-in | 248 B | Brief tone-before-voice behavior intended to open a repeater. Configured CTCSS remains. |
| V | Scan-speed telemetry | 208 B | Channels-per-second measurement/display. |
| W | Backlight fade | 204 B | Gradual fade; normal backlight operation remains. |
| X | TX voltage-sag display | 136 B | Sag measurement used on the About screen. Battery percentage/protection are separate. |

A module can affect several other features: removing gain/classification/quality
inputs can change adaptive squelch behavior. These interactions need explicit
implementation tests. Basic TX admission, battery safeguards, calibration,
PA shutdown and bootloader recovery are not proposed as expendable features.

## APRS-specific choices and display costs

These are also component-removal probes, not ready-to-flash cuts. Overlapping
rows must not be added together.

| ID | Component | Probe saving | Consequence |
|---|---|---:|---|
| Y | All APRS position decoding | 984 B | Received packets no longer yield coordinates; conflicts with the requested position display. |
| Y1 | Only compressed positions | 340 B | Stations sending base-91 compressed coordinates lose decoded positions; other formats remain. Included within Y. |
| Y2 | Only Mic-E positions | 288 B | Mic-E stations lose decoded positions; other formats remain. Included within Y. |
| Z | Static beacon encoder only | 536 B | Static beacon generation stops; messaging remains. Position/beacon setup cleanup could save additional bytes but is not measured here. |
| UI | All APRS setup drawing code | 1,740 B | Removes the setup display entirely, which is unusable without replacement. This is the current component cost, not a promised saving from a compact redesign. |

The editor, callsign entry, quick messages, position entry, status and radio
service share code. Removing one screen does not necessarily remove all of its
backend, and removing a common helper can affect more than one feature.

## Where the rest goes

The fitting development RX-session profile uses **61,368 bytes**; the complete
candidate adds **10,720 bytes** for the full messaging/setup/transmit paths.
This comparison includes shared-code changes and is not the cost of TX alone.
The minimal RX profile is **58,036 bytes**. These are different feature profiles,
not interchangeable claims of a finished transmit release.

For scale, the complete candidate's linked symbols include the following.
Inlining means these are compilation units of work, not clean feature budgets:

| Linked code/data | Bytes | Includes / qualification |
|---|---:|---|
| Main | 9,036 | Startup/main-loop work and inlined foreground handlers; not just the loop itself. |
| Main-screen rendering | 4,804 | Custom home, APRS browser/editor/setup drawing inlined into the display path. |
| Main radio key processing | 3,932 | Ordinary radio keypad operation and mode handling. |
| Settings display | 2,736 | Menu/value drawing; settings input/saving use additional code. |
| Packet decode | 2,516 | Packet decode/routing and inlined helpers. |
| Settings key handling | 1,696 | Navigating/editing settings; additional acceptance code is 1,574 B. |
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
RAM; cutting them is not a solution to a 10 KB flash shortage.

## Reproduction and next decision

The historical tables reproduce at the linked `aaef790` source with
`python3 tools/measure-features.py --transport`. On current source the same
command measures the new baseline; removed components no longer offer savings. It builds an enlarged-linker
baseline and each diagnostic variant in ignored copies, preserving original
source notices. `python3 tools/measure-transport.py` measures the unmodified
transport profile. No command here flashes hardware.

Select the IDs you can lose and any that must stay. We will implement and build
the combination before claiming that it fits. Savings are not additive under
LTO, and a fitting candidate still needs regression and physical RF validation.

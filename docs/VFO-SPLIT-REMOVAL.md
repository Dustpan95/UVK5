# Removed secondary-VFO background activity scan

The owner selected this removal after asking what the feature does. This is
VUURWERK's `vfo_split.c` engine, not repeater frequency offsets or ordinary A/B
VFO operation. Its original source/notices remain in the fork; the current
firmware does not link the engine (`ENABLE_VFO_SPLIT=0`).

The implemented intent is to operate on VFO A and periodically borrow the one
receiver to check memory channels through VFO B. It walks valid memories,
reads RSSI, alerts with a frequency toast and beep above its threshold, then
restores the voice VFO. It yields during TX, active reception, dual watch,
cross-band mode and key holds. This is not simultaneous dual reception.

**Only activity-alert mode is wired in this fork.** `SPLIT_AUTO_SWITCH` and
`SPLIT_LOG_ONLY` are enum names, but there are no corresponding implementation
branches or menu choices. The initial explanation overstated those capabilities;
source inspection corrected it before the removal.

What remains:

- Normal A/B VFO selection and the smaller secondary VFO home readout.
- Normal dual-watch alternation and cross-band RX/TX settings.
- Ordinary memory/channel and frequency scanning.
- Scan+Watch: OFF, SCAN A / WATCH B, SCAN B / WATCH A.
- Repeater frequency offsets and manual CTCSS/DCS settings.

The removed SPLIT option is absent from the available Scan+Watch choices.
The same choice table supplies both display and input limits; invalid removed
values are rejected on acceptance and safely displayed as OFF. The background
engine, activation state, periodic call and SP/activity overlays are excluded.
The complete candidate ELF contains no `gVfoSplit` or `VFO_SPLIT_Process`.

The full candidate saved **816 bytes**, from 68,920 to 68,104. Sharing existing
APRS setup handlers under pinned GCC LTO saved another **208 bytes** with the
same screens and controls. Current total is **67,896 / 61,440 bytes**: **6,456
still over flash**. Default RX is 54,164 and development RX-session 57,324.

Tests exercise the current disabled configuration, receiver arbitration,
retune/restore adapter, actual Scan+Watch options and retained tone settings.
No physical radio has been flashed or RF-tested. Preserving every remaining
feature in a fitting full-TX firmware is still an unresolved size problem;
small convenience cuts or an interface rewrite cannot be claimed to solve it
without a measured, validated build.

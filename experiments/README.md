# APRS TX size investigation

`aprs_tx_size.c` is a modified Apache-2.0 TA1JS prototype used only to measure
manual beacon integration size. It is not included in the default Makefile or
release image. Its dummy volatile inputs are not an operator configuration UI.
Do not flash or operate an investigation image.

Reproduce the isolated build from the repository root:

```bash
./tools/investigate-tx.sh
```

The script copies source into ignored `build/tx-investigation`, adds the trial
object, and makes the RxLog action reference it to keep the code reachable
under LTO. It uses the default 60 KiB linker limit and never flashes hardware.
The trial omits the callsign/location editor, message composer, scheduling,
ACK/retry logic and a correction for TA1JS's documented 1200/1800 Hz FFSK TX
compatibility issue. Only its encoded frame and register recipe are investigated;
no transmitted RF waveform or interoperability was tested.

See [the investigation report](../docs/APRS-TX-INVESTIGATION.md) for the result
and the remaining implementation/hardware work.

## Standard Bell 202 protocol helpers

`bell202_symbols.c` generates FCS/HDLC/NRZI tone selections and is independently
tested as ideal 1200/2200 Hz audio with `tools/test-bell202.py`. The actual
message and static-beacon builders are in `app/aprs_message.c`. There is no
working radio output or scheduler in these helpers.

`python3 tools/measure-protocol.py` measures the additional retained code in an
isolated diagnostic copy. Its callback writes synthetic tone words to volatile
RAM, not a radio. Outputs use a deliberately oversized linker region solely for
measurement and must never be flashed or released. It does not measure a full
TX service. See `docs/FLASH-FEASIBILITY.md` for exact totals and omissions.

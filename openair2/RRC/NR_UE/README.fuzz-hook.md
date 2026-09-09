# NR UE signaling hooks

The UE reads `/tmp/oai_nr_ue_hook_<ue-id>.ctl` and writes the matching `.state`
file. Hooks act when a message actually reaches the UE RRC handler or outgoing
SRB path; arming a hook does not generate a protocol procedure.

## Supported operations

Every target below supports `drop`, `delay`, `duplicate`, and `replay`.
Field operations are limited to registered adapters, not arbitrary ASN.1 paths.

| Target | Fields available to `mutate_field` |
| --- | --- |
| `RRCSetupComplete` | `transactionIdentifier` |
| `SecurityModeComplete` | `transactionIdentifier` |
| `RRCReconfigurationComplete` | `transactionIdentifier`, `logMeasAvailable`, `sigLogMeasConfigAvailable` |
| `RRCReestablishmentComplete` | `transactionIdentifier` |
| `UECapabilityInformation` | `transactionIdentifier` |
| `ULInformationTransfer` | `dedicatedNAS-Message` |
| `MeasurementReport` | Fields registered in the pipeline-generated adapter catalog |
| `DL_RRCReconfiguration` | `transactionIdentifier` |
| `DL_SecurityModeCommand` | `transactionIdentifier` |
| `DL_UECapabilityEnquiry` | `transactionIdentifier` |
| `DL_RRCReestablishment` | `transactionIdentifier` |
| `DL_RRCRelease` | `transactionIdentifier` |

UL fields change before ASN.1 encoding. DL fields change the decoded **local UE
object**; they do not change gNB transmissions or the original bytes used for
integrity verification. DL duplicate/replay reprocesses those original bytes
once, including normal handler side effects. Replay uses `replay_delay_ms`;
`replay_mode` is descriptive, not a scheduler for later protocol states.
Delays block the RRC task, including its event processing.

With `arm_once=1`, the first successful action disarms and removes the control
file. With `arm_once=0`, each new matching message triggers an action, but a
locally replayed copy never triggers another hook.

## Control examples

Minimal transaction mutation, compatible with the existing client `arm/oneshot`
commands (offsets wrap within 0–3):

```ini
enabled=1
target=RRCReconfigurationComplete
action=mutate_txn
arm_once=1
txn_offset=1
```

NAS replacement uses an explicit field contract:

```ini
enabled=1
target=ULInformationTransfer
action=mutate_field
arm_once=1
field_mutation_enabled=1
field_mutation_message=ULInformationTransfer
field_mutation_field=dedicatedNAS-Message
field_mutation_operator_family=optional_octet_string_assignment
field_mutation_selected_mode=force_ff
```

NAS modes are `force_zero`, `force_ff` (replace with one byte), and `omit`.
Transaction contracts use `integer_transform` and `mismatch_in_range`,
`boundary_min`, or `boundary_max`. For DL contracts, use the canonical `DL_`
message name in both `target` and `field_mutation_message`.

Check `hook_fire_count`, `last_hook_msg`, and `last_hook_action` in `.state`.
`seen_*` means observed by the hook, not successfully received by the gNB;
outgoing dropped messages are also observed.

## Regression tests

From the repository root, using the existing build directory:

```bash
cmake -S . -B cmake-build-debug -DENABLE_TESTS=ON
cmake --build cmake-build-debug --target nr_ue_fuzz_hook_test nr-uesoftmodem -j2
ctest --test-dir cmake-build-debug -R '^nr_ue_fuzz_hook_test$' --output-on-failure
```

The test exercises production hook helpers, private control/state files, and
real ASN.1 codecs with a counted PDCP sink. It does not replace RF simulator
or gNB/UE integration testing. Generated adapters remain owned by the research
pipeline and must not be edited manually.

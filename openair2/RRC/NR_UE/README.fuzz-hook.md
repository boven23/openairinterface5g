# NR UE signaling hooks

The UE reads `/tmp/oai_nr_ue_hook_<ue-id>.ctl` and writes the matching `.state`
file. Hooks act when a message actually reaches the UE RRC handler or outgoing
SRB path; arming a hook does not generate a protocol procedure.

## Supported operations

Every target below supports `drop`, `delay`, `duplicate`, and `replay`.
Field operations are limited to registered adapters, not arbitrary ASN.1 paths.

| Target | Fields available to `mutate_field` |
| --- | --- |
| `RRCSetupComplete` | `transactionIdentifier`, context-dependent `selectedPLMN-Identity` |
| `SecurityModeComplete` | `transactionIdentifier` |
| `RRCReconfigurationComplete` | `transactionIdentifier`, `logMeasAvailable`, `sigLogMeasConfigAvailable` |
| `RRCReestablishmentComplete` | `transactionIdentifier` |
| `UECapabilityInformation` | `transactionIdentifier`, context-dependent `ue-CapabilityRAT-ContainerList` |
| `ULInformationTransfer` | `dedicatedNAS-Message` |
| `MeasurementReport` | Generated field adapters, context-dependent `measId` / `measResults` |
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

## Downlink-dependent mutations

Use `action=mutate_field` and `field_mutation_transform=runtime_context` to
select native context adapters. Ordinary generated adapters keep their existing
behavior. All names in the table below are exact control-file values.

| Message | Field | Operator family | Selected mode |
| --- | --- | --- | --- |
| `MeasurementReport` | `measId` | `integer_transform` | `configured_measId_mismatch` |
| `MeasurementReport` | `measId` | `integer_transform` | `removed_measId_reuse` |
| `MeasurementReport` | `measResults` | `field_assignment` | `rsType_shape_mismatch` |
| `MeasurementReport` | `measResults` | `field_assignment` | `reportQuantity_mismatch` |
| `MeasurementReport` | `measResults` | `field_assignment` | `stale_report_after_reconfiguration` |
| `RRCSetupComplete` | `selectedPLMN-Identity` | `integer_transform` | `configured_plmn_mismatch` |
| `UECapabilityInformation` | `ue-CapabilityRAT-ContainerList` | `field_assignment` | `requested_rat_mismatch` |

For example, place these contents in the UE's control file before its next
measurement report. This does not require changing the research repositories:

```ini
enabled=1
target=MeasurementReport
action=mutate_field
arm_once=1
require_security=1
require_reconfiguration_complete=1
field_mutation_enabled=1
field_mutation_message=MeasurementReport
field_mutation_field=measId
field_mutation_operator_family=integer_transform
field_mutation_transform=runtime_context
field_mutation_selected_mode=configured_measId_mismatch
```

Measurement snapshots are taken **after** applying a measurement configuration,
per gNB context. They record effective ID-to-object/report bindings, RS type and
requested RSRP/RSRQ/SINR. Supported source configurations are NR periodical and
event-triggered reports. Source reports must agree with the modeled serving-cell
shape and quantities. Missing/unsupported context leaves the message unchanged
and the hook armed, without incrementing `hook_fire_count`.

The ID mismatch chooses a legal 1–64 value absent from the effective ID list;
all 64 values configured means no candidate. Removed-ID reuse requires an
observed deletion; re-adding an ID makes it ineligible. Shape mutation moves the
first serving-cell result to the other RS type. Quantity mutation removes one
requested quantity or adds one unrequested quantity using a legal value.

Stale-report mode replaces a newly generated report with a cached, unmodified
report from an earlier configuration. It requires proof that the old ID was
removed or the old serving-cell RS shape/quantities violate the current binding.
Renaming an otherwise equivalent report/object is not proof. Other context
changes, such as measurement frequency or event thresholds, are not modeled.
No new report means no opportunity to inject; there is no autonomous replay
scheduler. IDLE and RRCSetup fallback clear live measurement tables, caches and
completion observations, preventing reuse across connections.

The optional gates default to off. `require_security=1` requires local AS
security activation plus a successful SecurityModeComplete submission to PDCP.
`require_reconfiguration_complete=1` requires a successful completion submission
for the current measurement generation. PDCP rejection and hook drop do not
open these gates. Successful PDCP submission is **not** gNB receipt or acceptance.

Inspect `context_result` for `applied` or a skip/failure reason. State fields
`meas.<gnb>.generation`, `meas.<gnb>.completion_submitted_generation` and
`meas.<gnb>.<id>.{configured,supported,removed,object_id,report_id,rs_type,quantities}`
describe the snapshot. Quantity bits are RSRP=1, RSRQ=2, SINR=4; RS types are
SSB=0 and CSI-RS=1. `mutation_generation`, `original_meas_id`, `mutated_meas_id`
describe the last successful measurement mutation. `submitted.<message>` counts
successful PDCP submissions since connection-context reset.

PLMN mutation picks a value inside ASN.1's 1–12 range but outside the acquired
SIB1 list; it skips if the list is unavailable or already covers all values.
Capability mutation changes only the first container's outer RAT tag to an
unrequested RAT while retaining its bytes. The enclosing RRC message remains
encodable, but the retained NR blob need not decode as that other RAT. This
does not implement deep capability adapters, filter checking or segmentation.
Automatic case export and campaign oracles still belong to `OAI/` and
`OAI_fuzzer/` and are outside this repository's changes.

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

Context regressions cover all five measurement modes, equivalent-configuration
negative controls, missing/full/deleted/re-added ID sets, ID 64 removal, gNB
isolation, connection reset, failed first/duplicate/replay PDCP submissions,
security/configuration gates, PLMN and RAT mutations, and UPER round trips.

### Measurement source and bootstrap limits

Context mutation currently accepts only RSRP-only source configurations: the
production MeasurementReport constructor has no measured RSRQ/SINR input.
Other quantity combinations are rejected before source comparison with
`source_report_quantities_unsupported`. This is an explicit coverage limit;
no synthetic RSRQ/SINR is added to make a seed pass validation.

Bootstrap runs after the RRCReconfigurationComplete submission attempt, checks
both optional gates, and chooses an RSRP-only binding using its configured RS
type. Unsupported configurations or closed gates do not consume the attempt.
Successful hook execution and PDCP submission mark it done (drop needs no
submission). A later reconfiguration completion can retry an unfinished
bootstrap while the hook remains armed. Natural report scheduling is preserved.
Connection cleanup restores the same filter defaults as UE initialization.

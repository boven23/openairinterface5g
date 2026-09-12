/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* Exercise the production hooks and ASN.1 codecs with a counted PDCP sink.
 * Controls/state live in a private directory, never a running UE's /tmp files. */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include "NR_UL-DCCH-Message.h"
#include "uper_encoder.h"
#include "uper_decoder.h"
#include "../rrc_defs.h"
#include "common/utils/LOG/log.h"

#undef LOG_I
#undef LOG_W
#define LOG_I(component, ...)       \
  do {                              \
    if (0)                          \
      fprintf(stderr, __VA_ARGS__); \
  } while (0)
#define LOG_W(component, ...)       \
  do {                              \
    if (0)                          \
      fprintf(stderr, __VA_ARGS__); \
  } while (0)
static int sent_pdus;
static int fail_pdcp_call;
#define nr_pdcp_data_req_srb(...) (++sent_pdus != fail_pdcp_call)
#include "../nr_ue_fuzz_hook.inc.c"
#include "../nr_ue_meas_config.inc.c"

#define CHECK(condition)                                              \
  do {                                                                \
    if (!(condition)) {                                               \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
      abort();                                                        \
    }                                                                 \
  } while (0)

static char test_root[] = "/tmp/nr-ue-hook-test-XXXXXX";

static void reset(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, nr_ue_fuzz_hook_action_t action, bool once)
{
  memset(rrc, 0, sizeof(*rrc));
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  snprintf(hook->control_path, sizeof(hook->control_path), "%s/control", test_root);
  snprintf(hook->state_path, sizeof(hook->state_path), "%s/state", test_root);
  CHECK(unlink(hook->control_path) == 0 || errno == ENOENT);
  hook->control_mtime = -1;
  hook->control_mtime_nsec = -1;
  hook->enabled = true;
  hook->arm_once = once;
  hook->target_msg = msg;
  hook->action = action;
  hook->txn_offset = 1;
  hook->last_dl_txn = -1;
  hook->delay_ms = 1;
  hook->replay_delay_ms = 1;
  sent_pdus = 0;
  fail_pdcp_call = 0;
}

static void field_contract(NR_UE_RRC_INST_t *rrc, const char *field, const char *family, const char *mode)
{
  nr_ue_fuzz_hook_field_mutation_t *m = &rrc->fuzz_hook.field_mutation;
  m->enabled = true;
  nr_ue_fuzz_hook_copy_text(m->message, sizeof(m->message), nr_ue_fuzz_hook_msg_name(rrc->fuzz_hook.target_msg));
  nr_ue_fuzz_hook_copy_text(m->field, sizeof(m->field), field);
  nr_ue_fuzz_hook_copy_text(m->operator_family, sizeof(m->operator_family), family);
  nr_ue_fuzz_hook_copy_text(m->selected_mode, sizeof(m->selected_mode), mode);
}

static void test_dl_lifecycle(void)
{
  NR_UE_RRC_INST_t rrc;
  const nr_ue_fuzz_hook_action_t actions[] = {NR_UE_HOOK_ACTION_DROP,
                                              NR_UE_HOOK_ACTION_DELAY,
                                              NR_UE_HOOK_ACTION_DUPLICATE,
                                              NR_UE_HOOK_ACTION_REPLAY};
  for (int msg = NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION; msg <= NR_UE_HOOK_MSG_DL_RRC_RELEASE; msg++) {
    CHECK(nr_ue_fuzz_hook_msg_from_name(nr_ue_fuzz_hook_msg_name(msg)) == msg);
    for (size_t a = 0; a < sizeof(actions) / sizeof(*actions); a++) {
      for (int once = 0; once <= 1; once++) {
        reset(&rrc, msg, actions[a], once);
        for (int incoming = 0; incoming < 2; incoming++) {
          const bool active = !once || incoming == 0;
          bool drop = nr_ue_fuzz_hook_preprocess_dl(&rrc, msg, 2, false);
          CHECK(drop == (active && actions[a] == NR_UE_HOOK_ACTION_DROP));
          if (!drop) {
            bool repeat = nr_ue_fuzz_hook_should_repeat_dl(&rrc, msg, false);
            CHECK(repeat == (active && (actions[a] == NR_UE_HOOK_ACTION_DUPLICATE || actions[a] == NR_UE_HOOK_ACTION_REPLAY)));
            if (repeat) {
              CHECK(!nr_ue_fuzz_hook_preprocess_dl(&rrc, msg, 2, true));
              CHECK(!nr_ue_fuzz_hook_should_repeat_dl(&rrc, msg, true));
            }
          }
          CHECK(rrc.fuzz_hook.hook_fire_count == (once ? 1 : incoming + 1));
          CHECK(rrc.fuzz_hook.last_hook_msg == msg);
          CHECK(rrc.fuzz_hook.last_hook_action == actions[a]);
        }
        // A new controller action during a replay must not act on the local copy.
        rrc.fuzz_hook.enabled = true;
        rrc.fuzz_hook.action = NR_UE_HOOK_ACTION_DROP;
        CHECK(!nr_ue_fuzz_hook_preprocess_dl(&rrc, msg, 2, true));
        rrc.fuzz_hook.action = NR_UE_HOOK_ACTION_DUPLICATE;
        CHECK(!nr_ue_fuzz_hook_should_repeat_dl(&rrc, msg, true));
      }
    }
  }
}

static void test_ul_transport(void)
{
  NR_UE_RRC_INST_t rrc;
  uint8_t bytes[] = {1, 2, 3};
  const nr_ue_fuzz_hook_action_t actions[] = {NR_UE_HOOK_ACTION_DROP,
                                              NR_UE_HOOK_ACTION_DELAY,
                                              NR_UE_HOOK_ACTION_DUPLICATE,
                                              NR_UE_HOOK_ACTION_REPLAY};
  for (int msg = NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE; msg <= NR_UE_HOOK_MSG_MEASUREMENT_REPORT; msg++) {
    CHECK(nr_ue_fuzz_hook_msg_from_name(nr_ue_fuzz_hook_msg_name(msg)) == msg);
    for (size_t a = 0; a < sizeof(actions) / sizeof(*actions); a++) {
      for (int once = 0; once <= 1; once++) {
        reset(&rrc, msg, actions[a], once);
        nr_ue_fuzz_hook_send_srb(&rrc, msg, 1, bytes, sizeof(bytes));
        int expected = actions[a] == NR_UE_HOOK_ACTION_DROP ? 0 : actions[a] == NR_UE_HOOK_ACTION_DELAY ? 1 : 2;
        CHECK(sent_pdus == expected);
        CHECK(rrc.fuzz_hook.hook_fire_count == 1);
        CHECK(rrc.fuzz_hook.last_ul_msg == msg);
        CHECK(rrc.fuzz_hook.last_ul_size == sizeof(bytes));
        nr_ue_fuzz_hook_send_srb(&rrc, msg, 1, bytes, sizeof(bytes));
        CHECK(sent_pdus == expected + (once ? 1 : expected));
        CHECK(rrc.fuzz_hook.hook_fire_count == (once ? 1 : 2));
      }
    }
  }
}

/* All ten transaction-bearing payloads: legacy defaults, explicit contracts,
 * negative offsets, wrong target, and real UPER encode/decode round trips. */
#define TEST_TXN(type, id)                                                                                            \
  do {                                                                                                                \
    for (int explicit_contract = 0; explicit_contract <= 1; explicit_contract++) {                                    \
      reset(&rrc, id, explicit_contract ? NR_UE_HOOK_ACTION_MUTATE_FIELD : NR_UE_HOOK_ACTION_MUTATE_TXN, true);       \
      if (explicit_contract)                                                                                          \
        field_contract(&rrc, "transactionIdentifier", "integer_transform", "mismatch_in_range");                      \
      type##_t payload = {0};                                                                                         \
      payload.rrc_TransactionIdentifier = 0;                                                                          \
      payload.criticalExtensions.present = type##__criticalExtensions_PR_criticalExtensionsFuture;                    \
      payload.criticalExtensions.choice.criticalExtensionsFuture =                                                    \
          calloc(1, sizeof(*payload.criticalExtensions.choice.criticalExtensionsFuture));                             \
      rrc.fuzz_hook.txn_offset = -1;                                                                                  \
      rrc.fuzz_hook.last_dl_txn = 2;                                                                                  \
      CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_NONE, &payload));                                    \
      CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, id, &payload));                                                      \
      CHECK(payload.rrc_TransactionIdentifier == 3);                                                                  \
      CHECK(rrc.fuzz_hook.hook_fire_count == 1);                                                                      \
      CHECK(rrc.fuzz_hook.last_hook_action                                                                            \
            == (explicit_contract ? NR_UE_HOOK_ACTION_MUTATE_FIELD : NR_UE_HOOK_ACTION_MUTATE_TXN));                  \
      CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, id, &payload));                                                     \
      uint8_t bytes[64];                                                                                              \
      asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_##type, NULL, &payload, bytes, sizeof(bytes));              \
      CHECK(enc.encoded > 0);                                                                                         \
      type##_t *decoded = NULL;                                                                                       \
      asn_dec_rval_t dec = uper_decode(NULL, &asn_DEF_##type, (void **)&decoded, bytes, (enc.encoded + 7) / 8, 0, 0); \
      CHECK(dec.code == RC_OK);                                                                                       \
      CHECK(decoded->rrc_TransactionIdentifier == 3);                                                                 \
      ASN_STRUCT_FREE(asn_DEF_##type, decoded);                                                                       \
      ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_##type, &payload);                                                        \
    }                                                                                                                 \
  } while (0)

static void test_transactions(void)
{
  NR_UE_RRC_INST_t rrc;
  TEST_TXN(NR_RRCSetupComplete, NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE);
  TEST_TXN(NR_SecurityModeComplete, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE);
  TEST_TXN(NR_RRCReconfigurationComplete, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE);
  TEST_TXN(NR_RRCReestablishmentComplete, NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE);
  TEST_TXN(NR_UECapabilityInformation, NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION);
  TEST_TXN(NR_RRCReconfiguration, NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION);
  TEST_TXN(NR_SecurityModeCommand, NR_UE_HOOK_MSG_DL_SECURITY_MODE_COMMAND);
  TEST_TXN(NR_UECapabilityEnquiry, NR_UE_HOOK_MSG_DL_UE_CAPABILITY_ENQUIRY);
  TEST_TXN(NR_RRCReestablishment, NR_UE_HOOK_MSG_DL_RRC_REESTABLISHMENT);
  TEST_TXN(NR_RRCRelease, NR_UE_HOOK_MSG_DL_RRC_RELEASE);
}

static void test_legacy_control_and_ul_callback(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_NONE, NR_UE_HOOK_ACTION_NONE, false);
  FILE *ctl = fopen(rrc.fuzz_hook.control_path, "w");
  CHECK(ctl);
  CHECK(fputs("enabled=1\ntarget=RRCReconfigurationComplete\naction=mutate_txn\narm_once=1\ntxn_offset=1\n", ctl) >= 0);
  CHECK(fclose(ctl) == 0);
  NR_UL_DCCH_Message_t pdu = {0};
  struct NR_UL_DCCH_MessageType__c1 c1 = {0};
  NR_RRCReconfigurationComplete_t complete = {.rrc_TransactionIdentifier = 3};
  pdu.message.present = NR_UL_DCCH_MessageType_PR_c1;
  pdu.message.choice.c1 = &c1;
  c1.present = NR_UL_DCCH_MessageType__c1_PR_rrcReconfigurationComplete;
  c1.choice.rrcReconfigurationComplete = &complete;
  CHECK(nr_ue_fuzz_hook_mutate_ul_dcch(&rrc, &pdu));
  CHECK(complete.rrc_TransactionIdentifier == 0);
  CHECK(rrc.fuzz_hook.last_hook_action == NR_UE_HOOK_ACTION_MUTATE_TXN);
  CHECK(access(rrc.fuzz_hook.control_path, F_OK) == -1 && errno == ENOENT);
  CHECK(!nr_ue_fuzz_hook_mutate_ul_dcch(&rrc, &pdu));
  reset(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, NR_UE_HOOK_ACTION_MUTATE_TXN, false);
  field_contract(&rrc, "logMeasAvailable", "optional_presence_toggle", "force_present_true");
  CHECK(!nr_ue_fuzz_hook_mutate_ul_dcch(&rrc, &pdu));
  CHECK(rrc.fuzz_hook.hook_fire_count == 0);
}

static void test_nas_and_measurement_fields(void)
{
  NR_UE_RRC_INST_t rrc;
  const char *modes[] = {"force_zero", "force_ff", "omit"};
  for (size_t i = 0; i < sizeof(modes) / sizeof(*modes); i++) {
    reset(&rrc, NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
    field_contract(&rrc, "dedicatedNAS-Message", "optional_octet_string_assignment", modes[i]);
    NR_ULInformationTransfer_t payload = {0};
    payload.criticalExtensions.present = NR_ULInformationTransfer__criticalExtensions_PR_ulInformationTransfer;
    payload.criticalExtensions.choice.ulInformationTransfer = calloc(1, sizeof(NR_ULInformationTransfer_IEs_t));
    OCTET_STRING_t **nas = &payload.criticalExtensions.choice.ulInformationTransfer->dedicatedNAS_Message;
    *nas = calloc(1, sizeof(**nas));
    CHECK(OCTET_STRING_fromBuf(*nas, "nas", 3) == 0);
    CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER, &payload));
    uint8_t bytes[64];
    asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_ULInformationTransfer, NULL, &payload, bytes, sizeof(bytes));
    CHECK(enc.encoded > 0);
    NR_ULInformationTransfer_t *decoded = NULL;
    CHECK(uper_decode(NULL, &asn_DEF_NR_ULInformationTransfer, (void **)&decoded, bytes, (enc.encoded + 7) / 8, 0, 0).code
          == RC_OK);
    OCTET_STRING_t *result = decoded->criticalExtensions.choice.ulInformationTransfer->dedicatedNAS_Message;
    if (i == 2)
      CHECK(!result);
    else
      CHECK(result && result->size == 1 && result->buf[0] == (i == 0 ? 0 : 255));
    CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER, &payload));
    CHECK(rrc.fuzz_hook.hook_fire_count == 1);
    ASN_STRUCT_FREE(asn_DEF_NR_ULInformationTransfer, decoded);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_ULInformationTransfer, &payload);
  }
  reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, true);
  field_contract(&rrc, "measId", "integer_transform", "boundary_max");
  NR_MeasurementReport_t report = {0};
  NR_MeasurementReport_IEs_t ies = {0};
  report.criticalExtensions.present = NR_MeasurementReport__criticalExtensions_PR_measurementReport;
  report.criticalExtensions.choice.measurementReport = &ies;
  ies.measResults.measId = 1;
  CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, &report));
  CHECK(ies.measResults.measId == 64);
}

/* Preserve remote exact-path selection alongside local native adapters. */
static void test_adapter_identity_selection(void)
{
  NR_UE_RRC_INST_t rrc;
  for (int by_domain = 0; by_domain < 2; by_domain++) {
    reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
    field_contract(&rrc, "measId", "integer_transform", "boundary_max");
    nr_ue_fuzz_hook_field_mutation_t *m = &rrc.fuzz_hook.field_mutation;
    if (by_domain)
      nr_ue_fuzz_hook_copy_text(m->domain_id,
                                sizeof(m->domain_id),
                                "MeasurementReport__criticalExtensions__measurementReport__measResults__measId");
    else
      nr_ue_fuzz_hook_copy_text(m->adapter_key, sizeof(m->adapter_key), "7d63aed689d30406");
    NR_MeasurementReport_t report = {0};
    NR_MeasurementReport_IEs_t ies = {0};
    report.criticalExtensions.present = NR_MeasurementReport__criticalExtensions_PR_measurementReport;
    report.criticalExtensions.choice.measurementReport = &ies;
    ies.measResults.measId = 1;
    CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, &report));
    CHECK(ies.measResults.measId == 64);
    ies.measResults.measId = 1;
    nr_ue_fuzz_hook_copy_text(m->adapter_key, sizeof(m->adapter_key), "unknown-adapter");
    CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, &report));
    CHECK(ies.measResults.measId == 1);
    CHECK(rrc.fuzz_hook.hook_fire_count == 1);
  }
}

#include "nr_ue_fuzz_hook_context_test.inc.c"

/* Two controls in the same second must not reuse the previous action. */
static void test_subsecond_control_reload(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_NONE, NR_UE_HOOK_ACTION_NONE, false);
  for (int i = 0; i < 2; i++) {
    FILE *ctl = fopen(rrc.fuzz_hook.control_path, "w");
    CHECK(ctl);
    fprintf(ctl, "enabled=1\ntarget=RRCSetupComplete\naction=%s\n", i ? "duplicate" : "delay");
    CHECK(fclose(ctl) == 0);
    struct timespec stamps[2] = {{.tv_sec = 123456, .tv_nsec = i + 1}, {.tv_sec = 123456, .tv_nsec = i + 1}};
    CHECK(utimensat(AT_FDCWD, rrc.fuzz_hook.control_path, stamps, 0) == 0);
    nr_ue_fuzz_hook_reload_config(&rrc);
    CHECK(rrc.fuzz_hook.action == (i ? NR_UE_HOOK_ACTION_DUPLICATE : NR_UE_HOOK_ACTION_DELAY));
  }
  CHECK(unlink(rrc.fuzz_hook.control_path) == 0);
}

#include "nr_ue_fuzz_hook_python_fixture.inc.c"

int main(int argc, char **argv)
{
  logInit();
  if (argc == 5 && !strcmp(argv[1], "--control-fixture")) {
    int result = python_control_fixture(argv[2], argv[3], argv[4]);
    logTerm();
    return result;
  }
  CHECK(mkdtemp(test_root));
  test_dl_lifecycle();
  test_ul_transport();
  test_transactions();
  test_legacy_control_and_ul_callback();
  test_nas_and_measurement_fields();
  test_review_regressions();
  test_adapter_identity_selection();
  test_subsecond_control_reload();
  test_context_modes();
  test_context_lifecycle_and_removal();
  test_context_gates();
  test_context_control();
  test_context_capability_and_plmn();
  char path[512];
  snprintf(path, sizeof(path), "%s/state", test_root);
  CHECK(unlink(path) == 0);
  CHECK(rmdir(test_root) == 0);
  logTerm();
  puts("UE hook lifecycle, legacy control, transaction codecs and field adapters: PASS");
  return 0;
}

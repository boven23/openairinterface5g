/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */

static void context_contract(NR_UE_RRC_INST_t *rrc, const char *field, const char *family, const char *mode)
{
  field_contract(rrc, field, family, mode);
  nr_ue_fuzz_hook_copy_text(rrc->fuzz_hook.field_mutation.transform_name,
                            sizeof(rrc->fuzz_hook.field_mutation.transform_name),
                            "runtime_context");
}

static void configure_binding(rrcPerNB_t *nb, int id, int object, int report, int rs_type, unsigned quantities)
{
  CHECK(id >= 1 && id <= MAX_MEAS_ID);
  if (!nb->MeasId[id])
    nb->MeasId[id] = calloc(1, sizeof(*nb->MeasId[id]));
  *nb->MeasId[id] = (NR_MeasIdToAddMod_t){.measId = id, .measObjectId = object, .reportConfigId = report};
  if (!nb->MeasObj[object]) {
    nb->MeasObj[object] = calloc(1, sizeof(*nb->MeasObj[object]));
    nb->MeasObj[object]->measObjectId = object;
    nb->MeasObj[object]->measObject.present = NR_MeasObjectToAddMod__measObject_PR_measObjectNR;
    nb->MeasObj[object]->measObject.choice.measObjectNR = calloc(1, sizeof(NR_MeasObjectNR_t));
  }
  if (!nb->ReportConfig[report]) {
    nb->ReportConfig[report] = calloc(1, sizeof(*nb->ReportConfig[report]));
    nb->ReportConfig[report]->reportConfigId = report;
    nb->ReportConfig[report]->reportConfig.present = NR_ReportConfigToAddMod__reportConfig_PR_reportConfigNR;
    nb->ReportConfig[report]->reportConfig.choice.reportConfigNR = calloc(1, sizeof(NR_ReportConfigNR_t));
    NR_ReportConfigNR_t *nr = nb->ReportConfig[report]->reportConfig.choice.reportConfigNR;
    nr->reportType.present = NR_ReportConfigNR__reportType_PR_periodical;
    nr->reportType.choice.periodical = calloc(1, sizeof(NR_PeriodicalReportConfig_t));
  }
  NR_PeriodicalReportConfig_t *periodic =
      nb->ReportConfig[report]->reportConfig.choice.reportConfigNR->reportType.choice.periodical;
  periodic->rsType = rs_type;
  periodic->reportQuantityCell.rsrp = (quantities & 1) != 0;
  periodic->reportQuantityCell.rsrq = (quantities & 2) != 0;
  periodic->reportQuantityCell.sinr = (quantities & 4) != 0;
}

static NR_MeasurementReport_t *make_report(int id, int rs_type, unsigned quantities)
{
  NR_MeasurementReport_t *report = calloc(1, sizeof(*report));
  report->criticalExtensions.present = NR_MeasurementReport__criticalExtensions_PR_measurementReport;
  report->criticalExtensions.choice.measurementReport = calloc(1, sizeof(NR_MeasurementReport_IEs_t));
  NR_MeasResults_t *results = nr_ue_hook_meas_results(report);
  results->measId = id;
  NR_MeasResultServMO_t *serving = calloc(1, sizeof(*serving));
  serving->servCellId = 0;
  NR_MeasQuantityResults_t *q = calloc(1, sizeof(*q));
  if (quantities & 1) {
    q->rsrp = calloc(1, sizeof(*q->rsrp));
    *q->rsrp = 50;
  }
  if (quantities & 2) {
    q->rsrq = calloc(1, sizeof(*q->rsrq));
    *q->rsrq = 20;
  }
  if (quantities & 4) {
    q->sinr = calloc(1, sizeof(*q->sinr));
    *q->sinr = 30;
  }
  if (rs_type == NR_NR_RS_Type_ssb)
    serving->measResultServingCell.measResult.cellResults.resultsSSB_Cell = q;
  else
    serving->measResultServingCell.measResult.cellResults.resultsCSI_RS_Cell = q;
  CHECK(ASN_SEQUENCE_ADD(&results->measResultServingMOList.list, serving) == 0);
  return report;
}

static NR_MeasurementReport_t *roundtrip_report(NR_MeasurementReport_t *report)
{
  uint8_t bytes[NR_RRC_BUF_SIZE];
  asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_MeasurementReport, NULL, report, bytes, sizeof(bytes));
  CHECK(enc.encoded > 0);
  NR_MeasurementReport_t *decoded = NULL;
  CHECK(uper_decode(NULL, &asn_DEF_NR_MeasurementReport, (void **)&decoded, bytes, (enc.encoded + 7) / 8, 0, 0).code == RC_OK);
  return decoded;
}

static bool mutate_report(NR_UE_RRC_INST_t *rrc, NR_MeasurementReport_t *report)
{
  NR_UL_DCCH_Message_t pdu = {0};
  struct NR_UL_DCCH_MessageType__c1 c1 = {.present = NR_UL_DCCH_MessageType__c1_PR_measurementReport};
  c1.choice.measurementReport = report;
  pdu.message.present = NR_UL_DCCH_MessageType_PR_c1;
  pdu.message.choice.c1 = &c1;
  return nr_ue_fuzz_hook_mutate_ul_dcch(rrc, &pdu);
}

static void test_context_modes(void)
{
  NR_UE_RRC_INST_t rrc;
  const char *modes[] = {"configured_measId_mismatch", "removed_measId_reuse", "rsType_shape_mismatch", "reportQuantity_mismatch"};
  for (int rs = NR_NR_RS_Type_ssb; rs <= NR_NR_RS_Type_csi_rs; rs++) {
    for (int mode = 0; mode < 4; mode++) {
      reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
      context_contract(&rrc, mode < 2 ? "measId" : "measResults", mode < 2 ? "integer_transform" : "field_assignment", modes[mode]);
      NR_MeasurementReport_t *report = make_report(1, rs, 1);
      CHECK(!mutate_report(&rrc, report)); // receiving a DL message alone is not applied context
      CHECK(rrc.fuzz_hook.hook_fire_count == 0);
      configure_binding(&rrc.perNB[0], 1, 1, 1, rs, 1);
      configure_binding(&rrc.perNB[0], 64, 64, 64, rs, 1);
      nr_ue_hook_snapshot_meas(&rrc, 0);
      NR_MeasurementReport_t *control = roundtrip_report(report);
      CHECK(nr_ue_hook_report_matches(nr_ue_hook_meas_results(control), &rrc.perNB[0].hook_meas.binding[1]));
      ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, control);
      if (mode == 1) {
        CHECK(!mutate_report(&rrc, report)); // no historical removal: no invented old ID
        long removed = 64;
        NR_MeasIdToRemoveList_t list = {0};
        CHECK(ASN_SEQUENCE_ADD(&list.list, &removed) == 0);
        handle_measid_remove(&rrc.perNB[0], &list, &rrc.timers_and_constants);
        free(list.list.array);
        nr_ue_hook_snapshot_meas(&rrc, 0);
      }
      CHECK(mutate_report(&rrc, report));
      CHECK(rrc.fuzz_hook.hook_fire_count == 1);
      NR_MeasurementReport_t *decoded = roundtrip_report(report);
      NR_MeasResults_t *result = nr_ue_hook_meas_results(decoded);
      if (mode < 2)
        CHECK(result->measId == (mode == 0 ? 2 : 64));
      else
        CHECK(!nr_ue_hook_report_matches(result, &rrc.perNB[0].hook_meas.binding[1]));
      CHECK(rrc.fuzz_hook.mutation_generation == rrc.perNB[0].hook_meas.generation);
      ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, decoded);
      ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
      nr_ue_hook_clear_context(&rrc);
    }
  }
  reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
  context_contract(&rrc, "measId", "integer_transform", "configured_measId_mismatch");
  for (int id = 1; id <= 64; id++)
    configure_binding(&rrc.perNB[0], id, 1, 1, NR_NR_RS_Type_ssb, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  NR_MeasurementReport_t *report = make_report(64, NR_NR_RS_Type_ssb, 1);
  CHECK(!mutate_report(&rrc, report)); // all 64 values are configured
  CHECK(nr_ue_hook_meas_results(report)->measId == 64);
  CHECK(rrc.fuzz_hook.hook_fire_count == 0 && rrc.fuzz_hook.enabled);
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
  nr_ue_hook_clear_context(&rrc);

  // Equivalent configuration renaming is a negative control for stale replay.
  reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
  context_contract(&rrc, "measResults", "field_assignment", "stale_report_after_reconfiguration");
  configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_ssb, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  report = make_report(1, NR_NR_RS_Type_ssb, 1);
  CHECK(!mutate_report(&rrc, report)); // caches a normal seed, no old generation yet
  configure_binding(&rrc.perNB[0], 1, 2, 2, NR_NR_RS_Type_ssb, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  CHECK(!mutate_report(&rrc, report));
  CHECK(rrc.fuzz_hook.hook_fire_count == 0);
  configure_binding(&rrc.perNB[0], 1, 2, 2, NR_NR_RS_Type_csi_rs, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
  report = make_report(1, NR_NR_RS_Type_csi_rs, 1);
  CHECK(mutate_report(&rrc, report));
  NR_MeasurementReport_t *decoded = roundtrip_report(report);
  CHECK(!nr_ue_hook_report_matches(nr_ue_hook_meas_results(decoded), &rrc.perNB[0].hook_meas.binding[1]));
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, decoded);
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
  nr_ue_hook_clear_context(&rrc);
}

static void test_context_lifecycle_and_removal(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
  configure_binding(&rrc.perNB[0], 64, 64, 64, NR_NR_RS_Type_ssb, 1);
  configure_binding(&rrc.perNB[1], 64, 64, 64, NR_NR_RS_Type_csi_rs, 7);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  nr_ue_hook_snapshot_meas(&rrc, 1);
  long removed = 64;
  NR_MeasObjectToRemoveList_t list = {0};
  CHECK(ASN_SEQUENCE_ADD(&list.list, &removed) == 0);
  handle_measobj_remove(&rrc.perNB[0], &list, &rrc.timers_and_constants);
  free(list.list.array);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  CHECK(!rrc.perNB[0].MeasObj[64] && !rrc.perNB[0].MeasId[64]);
  CHECK(rrc.perNB[0].hook_meas.removed[64]);
  CHECK(rrc.perNB[1].MeasId[64] && rrc.perNB[1].hook_meas.binding[64].supported);
  configure_binding(&rrc.perNB[0], 64, 64, 64, NR_NR_RS_Type_ssb, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  CHECK(!rrc.perNB[0].hook_meas.removed[64]); // re-added IDs cannot be selected as removed
  unsigned long generation = rrc.perNB[0].hook_meas.generation;
  for (int update = 0; update < 2; update++) {
    NR_QuantityConfig_t quantity = {0};
    quantity.quantityConfigNR_List = calloc(1, sizeof(*quantity.quantityConfigNR_List));
    NR_QuantityConfigNR_t *entry = calloc(1, sizeof(*entry));
    entry->quantityConfigCell.ssb_FilterConfig.filterCoefficientRSRP = calloc(1, sizeof(long));
    *entry->quantityConfigCell.ssb_FilterConfig.filterCoefficientRSRP = 4 + update;
    CHECK(ASN_SEQUENCE_ADD(&quantity.quantityConfigNR_List->list, entry) == 0);
    handle_quantityconfig(&rrc.perNB[0], &quantity, &rrc.timers_and_constants);
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_QuantityConfig, &quantity);
    CHECK(*rrc.perNB[0].QuantityConfig[0]->quantityConfigCell.ssb_FilterConfig.filterCoefficientRSRP == 4 + update);
  }
  nr_ue_hook_clear_context(&rrc);
  CHECK(!rrc.perNB[0].hook_meas.valid && rrc.perNB[0].hook_meas.generation > generation);
  CHECK(!rrc.perNB[1].MeasId[64]);
  nr_ue_hook_snapshot_meas(&rrc, 0); // an empty delta in a new connection cannot resurrect old IDs
  CHECK(!rrc.perNB[0].hook_meas.binding[64].configured);
  nr_ue_hook_clear_context(&rrc);
}

static void test_context_gates(void)
{
  NR_UE_RRC_INST_t rrc;
  uint8_t bytes[] = {1};
  for (int action = NR_UE_HOOK_ACTION_NONE; action <= NR_UE_HOOK_ACTION_DELAY; action++) {
    reset(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, action, true);
    configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_ssb, 1);
    nr_ue_hook_snapshot_meas(&rrc, 0);
    fail_pdcp_call = 1;
    nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, 1, bytes, sizeof(bytes));
    CHECK(rrc.fuzz_hook.submitted[NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE] == 0);
    CHECK(rrc.perNB[0].hook_meas.completion_submitted_generation == 0);
    rrc.fuzz_hook.require_reconfiguration_complete = true;
    CHECK(!nr_ue_hook_gates_ready(&rrc));
    rrc.fuzz_hook.require_reconfiguration_complete = false;
    rrc.fuzz_hook.action = NR_UE_HOOK_ACTION_NONE;
    fail_pdcp_call = 0;
    nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, 1, bytes, sizeof(bytes));
    rrc.fuzz_hook.require_reconfiguration_complete = true;
    CHECK(nr_ue_hook_gates_ready(&rrc));
    nr_ue_hook_snapshot_meas(&rrc, 0);
    CHECK(!nr_ue_hook_gates_ready(&rrc));
    nr_ue_hook_clear_context(&rrc);
  }
  for (int action = NR_UE_HOOK_ACTION_DUPLICATE; action <= NR_UE_HOOK_ACTION_REPLAY; action++) {
    reset(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, action, true);
    fail_pdcp_call = 2;
    nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, 1, bytes, sizeof(bytes));
    CHECK(rrc.fuzz_hook.submitted[NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE] == 1);
    CHECK(rrc.fuzz_hook.hook_fire_count == 0 && rrc.fuzz_hook.enabled);
  }
  reset(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, NR_UE_HOOK_ACTION_NONE, false);
  rrc.fuzz_hook.require_security = true;
  rrc.as_security_activated = true;
  fail_pdcp_call = 1;
  nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, 1, bytes, sizeof(bytes));
  CHECK(!nr_ue_hook_gates_ready(&rrc));
  fail_pdcp_call = 0;
  nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, 1, bytes, sizeof(bytes));
  CHECK(nr_ue_hook_gates_ready(&rrc));
  rrc.as_security_activated = false;
  CHECK(!nr_ue_hook_gates_ready(&rrc));
}

static void test_context_control(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_NONE, NR_UE_HOOK_ACTION_NONE, false);
  configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_ssb, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  FILE *ctl = fopen(rrc.fuzz_hook.control_path, "w");
  CHECK(ctl);
  CHECK(fputs("enabled=1\ntarget=MeasurementReport\naction=mutate_field\narm_once=1\n"
              "require_security=1\nrequire_reconfiguration_complete=1\n"
              "field_mutation_enabled=1\nfield_mutation_message=MeasurementReport\n"
              "field_mutation_field=measId\nfield_mutation_operator_family=integer_transform\n"
              "field_mutation_transform=runtime_context\n"
              "field_mutation_selected_mode=configured_measId_mismatch\n",
              ctl)
        >= 0);
  CHECK(fclose(ctl) == 0);
  NR_MeasurementReport_t *report = make_report(1, NR_NR_RS_Type_ssb, 1);
  CHECK(!mutate_report(&rrc, report));
  CHECK(rrc.fuzz_hook.require_security && rrc.fuzz_hook.require_reconfiguration_complete);
  CHECK(!strcmp(rrc.fuzz_hook.context_result, "security_gate_not_ready"));
  uint8_t bytes[] = {1};
  rrc.as_security_activated = true;
  nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, 1, bytes, sizeof(bytes));
  CHECK(!mutate_report(&rrc, report));
  CHECK(!strcmp(rrc.fuzz_hook.context_result, "reconfiguration_gate_not_ready"));
  nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, 1, bytes, sizeof(bytes));
  CHECK(mutate_report(&rrc, report));
  CHECK(access(rrc.fuzz_hook.control_path, F_OK) < 0 && errno == ENOENT);
  FILE *state = fopen(rrc.fuzz_hook.state_path, "r");
  CHECK(state);
  bool applied = false, original = false, changed = false, submitted = false;
  char line[256];
  while (fgets(line, sizeof(line), state)) {
    applied |= !strcmp(line, "context_result=applied\n");
    original |= !strcmp(line, "original_meas_id=1\n");
    changed |= !strcmp(line, "mutated_meas_id=2\n");
    submitted |= !strcmp(line, "submitted.RRCReconfigurationComplete=1\n");
  }
  CHECK(fclose(state) == 0);
  CHECK(applied && original && changed && submitted);
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
  nr_ue_hook_clear_context(&rrc);
}

static void test_context_capability_and_plmn(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
  context_contract(&rrc, "ue-CapabilityRAT-ContainerList", "field_assignment", "requested_rat_mismatch");
  NR_UECapabilityEnquiry_t enquiry = {0};
  enquiry.criticalExtensions.present = NR_UECapabilityEnquiry__criticalExtensions_PR_ueCapabilityEnquiry;
  enquiry.criticalExtensions.choice.ueCapabilityEnquiry = calloc(1, sizeof(NR_UECapabilityEnquiry_IEs_t));
  NR_UE_CapabilityRAT_Request_t *request = calloc(1, sizeof(*request));
  request->rat_Type = NR_RAT_Type_nr;
  CHECK(ASN_SEQUENCE_ADD(&enquiry.criticalExtensions.choice.ueCapabilityEnquiry->ue_CapabilityRAT_RequestList.list, request) == 0);
  NR_UECapabilityInformation_t info = {0};
  info.criticalExtensions.present = NR_UECapabilityInformation__criticalExtensions_PR_ueCapabilityInformation;
  info.criticalExtensions.choice.ueCapabilityInformation = calloc(1, sizeof(NR_UECapabilityInformation_IEs_t));
  NR_UE_CapabilityRAT_ContainerList_t *list = calloc(1, sizeof(*list));
  info.criticalExtensions.choice.ueCapabilityInformation->ue_CapabilityRAT_ContainerList = list;
  NR_UE_CapabilityRAT_Container_t *container = calloc(1, sizeof(*container));
  container->rat_Type = NR_RAT_Type_nr;
  CHECK(OCTET_STRING_fromBuf(&container->ue_CapabilityRAT_Container, "nr", 2) == 0);
  CHECK(ASN_SEQUENCE_ADD(&list->list, container) == 0);
  CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION, &info));
  nr_ue_hook_record_enquiry(&rrc, &enquiry);
  CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION, &info));
  CHECK(!(rrc.perNB[0].hook_requested_rats & (1u << container->rat_Type)));
  CHECK(container->ue_CapabilityRAT_Container.size == 2 && !memcmp(container->ue_CapabilityRAT_Container.buf, "nr", 2));
  uint8_t bytes[256];
  asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_UECapabilityInformation, NULL, &info, bytes, sizeof(bytes));
  CHECK(enc.encoded > 0);
  NR_UECapabilityInformation_t *decoded = NULL;
  CHECK(uper_decode(NULL, &asn_DEF_NR_UECapabilityInformation, (void **)&decoded, bytes, (enc.encoded + 7) / 8, 0, 0).code
        == RC_OK);
  CHECK(decoded->criticalExtensions.choice.ueCapabilityInformation->ue_CapabilityRAT_ContainerList->list.array[0]->rat_Type
        == container->rat_Type);
  ASN_STRUCT_FREE(asn_DEF_NR_UECapabilityInformation, decoded);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_UECapabilityInformation, &info);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_UECapabilityEnquiry, &enquiry);
  nr_ue_hook_clear_context(&rrc);
  CHECK(!rrc.perNB[0].hook_capability_enquiry_valid);

  reset(&rrc, NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE, NR_UE_HOOK_ACTION_MUTATE_FIELD, true);
  context_contract(&rrc, "selectedPLMN-Identity", "integer_transform", "configured_plmn_mismatch");
  NR_RRCSetupComplete_t setup = {0};
  setup.criticalExtensions.present = NR_RRCSetupComplete__criticalExtensions_PR_rrcSetupComplete;
  setup.criticalExtensions.choice.rrcSetupComplete = calloc(1, sizeof(NR_RRCSetupComplete_IEs_t));
  setup.criticalExtensions.choice.rrcSetupComplete->selectedPLMN_Identity = 1;
  CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE, &setup));
  rrc.perNB[0].SInfo.sib1_validity = true;
  rrc.perNB[0].hook_plmn_count = 12;
  CHECK(!nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE, &setup));
  rrc.perNB[0].hook_plmn_count = 2;
  CHECK(nr_ue_fuzz_hook_mutate_payload(&rrc, NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE, &setup));
  CHECK(setup.criticalExtensions.choice.rrcSetupComplete->selectedPLMN_Identity == 3);
  enc = uper_encode_to_buffer(&asn_DEF_NR_RRCSetupComplete, NULL, &setup, bytes, sizeof(bytes));
  CHECK(enc.encoded > 0);
  NR_RRCSetupComplete_t *setup_decoded = NULL;
  CHECK(uper_decode(NULL, &asn_DEF_NR_RRCSetupComplete, (void **)&setup_decoded, bytes, (enc.encoded + 7) / 8, 0, 0).code == RC_OK);
  CHECK(setup_decoded->criticalExtensions.choice.rrcSetupComplete->selectedPLMN_Identity == 3);
  ASN_STRUCT_FREE(asn_DEF_NR_RRCSetupComplete, setup_decoded);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_RRCSetupComplete, &setup);
}

/* Exercise bootstrap with the production constructor, hooks and codecs. */
#include "RRC/NR/MESSAGES/asn1_msg.h"
static void bootstrap_generate(rrcPerNB_t *nb, instance_t ue_id);
#define rrc_ue_generate_measurementReport bootstrap_generate
#include "../nr_ue_fuzz_hook_bootstrap.inc.c"
#undef rrc_ue_generate_measurementReport
static NR_UE_RRC_INST_t *bootstrap_rrc;
static int bootstrap_calls;
static void bootstrap_generate(rrcPerNB_t *nb, instance_t ue_id)
{
  (void)ue_id;
  bootstrap_calls++;
  CHECK(nb->l3_measurements.rs_type == NR_NR_RS_Type_csi_rs);
  uint8_t bytes[NR_RRC_BUF_SIZE];
  int size = do_nrMeasurementReport_SA(nb->l3_measurements.trigger_to_measid,
                                       nb->l3_measurements.trigger_quantity,
                                       nb->l3_measurements.rs_type,
                                       0,
                                       50,
                                       false,
                                       0,
                                       50,
                                       bytes,
                                       sizeof(bytes),
                                       bootstrap_rrc,
                                       nr_ue_fuzz_hook_mutate_ul_dcch);
  CHECK(size > 0);
  nr_ue_fuzz_hook_send_srb(bootstrap_rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, 1, bytes, size);
}

static void test_review_regressions(void)
{
  NR_UE_RRC_INST_t rrc;
  reset(&rrc, NR_UE_HOOK_MSG_MEASUREMENT_REPORT, NR_UE_HOOK_ACTION_MUTATE_FIELD, false);
  nr_ue_hook_clear_context(&rrc);
  CHECK(rrc.perNB[0].l3_measurements.ssb_filter_coeff_rsrp == 1.0f);
  CHECK(rrc.perNB[0].l3_measurements.csi_RS_filter_coeff_rsrp == 1.0f);
  context_contract(&rrc, "measId", "integer_transform", "configured_measId_mismatch");
  configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_csi_rs, 3);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  NR_MeasurementReport_t *report = make_report(1, NR_NR_RS_Type_csi_rs, 1);
  CHECK(!mutate_report(&rrc, report));
  CHECK(!strcmp(rrc.fuzz_hook.context_result, "source_report_quantities_unsupported"));
  ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, report);
  rrc.fuzz_hook.measurement_bootstrap_enabled = true;
  bootstrap_rrc = &rrc;
  bootstrap_calls = 0;
  nr_ue_fuzz_hook_bootstrap_measurement_report(&rrc, 0);
  CHECK(bootstrap_calls == 0 && !rrc.fuzz_hook.measurement_bootstrap_done);
  configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_csi_rs, 1);
  nr_ue_hook_snapshot_meas(&rrc, 0);
  rrc.fuzz_hook.require_reconfiguration_complete = true;
  nr_ue_fuzz_hook_bootstrap_measurement_report(&rrc, 0);
  CHECK(bootstrap_calls == 0 && !rrc.fuzz_hook.measurement_bootstrap_done);
  uint8_t bytes[] = {1};
  nr_ue_fuzz_hook_send_srb(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, 1, bytes, sizeof(bytes));
  fail_pdcp_call = sent_pdus + 1;
  nr_ue_fuzz_hook_bootstrap_measurement_report(&rrc, 0);
  CHECK(bootstrap_calls == 1 && !rrc.fuzz_hook.measurement_bootstrap_done);
  fail_pdcp_call = 0;
  nr_ue_fuzz_hook_bootstrap_measurement_report(&rrc, 0);
  CHECK(bootstrap_calls == 2 && rrc.fuzz_hook.measurement_bootstrap_done);
  CHECK(rrc.perNB[0].l3_measurements.trigger_to_measid == 0);
  nr_ue_fuzz_hook_bootstrap_measurement_report(&rrc, 0);
  CHECK(bootstrap_calls == 2);
  nr_ue_hook_clear_context(&rrc);
}

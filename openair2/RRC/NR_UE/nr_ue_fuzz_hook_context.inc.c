/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* Native context adapters; do not add runtime semantics to generated adapters. */

static rrcPerNB_t *nr_ue_hook_context_nb(NR_UE_RRC_INST_t *rrc)
{
  int index = rrc->fuzz_hook.context_gnb;
  return index >= 0 && index < NB_CNX_UE ? &rrc->perNB[index] : NULL;
}

static bool nr_ue_hook_context_result(NR_UE_RRC_INST_t *rrc, const char *reason)
{
  nr_ue_fuzz_hook_copy_text(rrc->fuzz_hook.context_result, sizeof(rrc->fuzz_hook.context_result), reason);
  return false;
}

static unsigned nr_ue_hook_quantities(const NR_MeasReportQuantity_t *q)
{
  return (q->rsrp ? 1 : 0) | (q->rsrq ? 2 : 0) | (q->sinr ? 4 : 0);
}

/* Match the production report constructor: only measured RSRP is available. */
static bool nr_ue_hook_source_supported(const nr_ue_hook_meas_binding_t *binding)
{
  return binding->supported && binding->quantities == 1;
}

static void nr_ue_hook_reset_measurements(l3_measurements_t *meas)
{
  memset(meas, 0, sizeof(*meas));
  meas->ssb_filter_coeff_rsrp = 1.0f;
  meas->csi_RS_filter_coeff_rsrp = 1.0f;
}

static void nr_ue_hook_snapshot_meas(NR_UE_RRC_INST_t *rrc, int gnb)
{
  if (gnb < 0 || gnb >= NB_CNX_UE)
    return;
  rrcPerNB_t *nb = &rrc->perNB[gnb];
  nr_ue_hook_meas_context_t *ctx = &nb->hook_meas;
  rrc->fuzz_hook.context_gnb = gnb;
  if (ctx->current_report.size > 0)
    ctx->previous_report = ctx->current_report;
  ctx->current_report.size = 0;
  ctx->generation++;
  ctx->completion_submitted_generation = 0;
  for (int id = 1; id <= MAX_MEAS_ID; id++) {
    nr_ue_hook_meas_binding_t next = {0};
    const NR_MeasIdToAddMod_t *binding = nb->MeasId[id];
    if (binding && binding->measId == id) {
      next.configured = true;
      next.object_id = binding->measObjectId;
      next.report_id = binding->reportConfigId;
      if (next.object_id >= 1 && next.object_id <= MAX_MEAS_OBJ && next.report_id >= 1 && next.report_id <= MAX_MEAS_CONFIG
          && nb->MeasObj[next.object_id] && nb->ReportConfig[next.report_id]) {
        const NR_ReportConfigToAddMod_t *report = nb->ReportConfig[next.report_id];
        if (nb->MeasObj[next.object_id]->measObject.present == NR_MeasObjectToAddMod__measObject_PR_measObjectNR
            && report->reportConfig.present == NR_ReportConfigToAddMod__reportConfig_PR_reportConfigNR
            && report->reportConfig.choice.reportConfigNR) {
          const NR_ReportConfigNR_t *nr = report->reportConfig.choice.reportConfigNR;
          if (nr->reportType.present == NR_ReportConfigNR__reportType_PR_periodical && nr->reportType.choice.periodical) {
            next.supported = true;
            next.rs_type = nr->reportType.choice.periodical->rsType;
            next.quantities = nr_ue_hook_quantities(&nr->reportType.choice.periodical->reportQuantityCell);
          } else if (nr->reportType.present == NR_ReportConfigNR__reportType_PR_eventTriggered
                     && nr->reportType.choice.eventTriggered) {
            next.supported = true;
            next.rs_type = nr->reportType.choice.eventTriggered->rsType;
            next.quantities = nr_ue_hook_quantities(&nr->reportType.choice.eventTriggered->reportQuantityCell);
          }
          next.supported = next.supported && (next.rs_type == NR_NR_RS_Type_ssb || next.rs_type == NR_NR_RS_Type_csi_rs);
        }
      }
    }
    if (ctx->binding[id].configured && !next.configured)
      ctx->removed[id] = true;
    if (next.configured)
      ctx->removed[id] = false;
    ctx->binding[id] = next;
  }
  ctx->valid = true;
}

static void nr_ue_hook_clear_context(NR_UE_RRC_INST_t *rrc)
{
  // Called only when releasing the connection configuration (IDLE or Setup
  // fallback). Clear the live snapshot sources as well as hook observations.
  for (int gnb = 0; gnb < NB_CNX_UE; gnb++) {
    rrcPerNB_t *nb = &rrc->perNB[gnb];
    for (int id = 1; id <= MAX_MEAS_ID; id++) {
      ASN_STRUCT_FREE(asn_DEF_NR_MeasIdToAddMod, nb->MeasId[id]);
      nb->MeasId[id] = NULL;
      ASN_STRUCT_FREE(asn_DEF_NR_VarMeasReport, nb->MeasReport[id]);
      nb->MeasReport[id] = NULL;
    }
    for (int id = 1; id <= MAX_MEAS_OBJ; id++) {
      ASN_STRUCT_FREE(asn_DEF_NR_MeasObjectToAddMod, nb->MeasObj[id]);
      nb->MeasObj[id] = NULL;
    }
    for (int id = 1; id <= MAX_MEAS_CONFIG; id++) {
      ASN_STRUCT_FREE(asn_DEF_NR_ReportConfigToAddMod, nb->ReportConfig[id]);
      nb->ReportConfig[id] = NULL;
    }
    for (int id = 0; id < MAX_QUANTITY_CONFIG; id++) {
      ASN_STRUCT_FREE(asn_DEF_NR_QuantityConfigNR, nb->QuantityConfig[id]);
      nb->QuantityConfig[id] = NULL;
    }
    ASN_STRUCT_FREE(asn_DEF_NR_MeasGapConfig, nb->measGapConfig);
    nb->measGapConfig = NULL;
    nr_ue_hook_reset_measurements(&nb->l3_measurements);
    nr_ue_hook_meas_context_t *ctx = &rrc->perNB[gnb].hook_meas;
    unsigned long generation = ctx->generation + 1;
    memset(ctx, 0, sizeof(*ctx));
    ctx->generation = generation;
    rrc->perNB[gnb].hook_capability_enquiry_valid = false;
    rrc->perNB[gnb].hook_requested_rats = 0;
  }
  memset(rrc->fuzz_hook.submitted, 0, sizeof(rrc->fuzz_hook.submitted));
  nr_timer_stop(&rrc->timers_and_constants.T321);
  rrc->fuzz_hook.mutation_generation = 0;
  rrc->fuzz_hook.original_meas_id = 0;
  rrc->fuzz_hook.mutated_meas_id = 0;
  nr_ue_hook_context_result(rrc, "connection_context_cleared");
}

static bool nr_ue_hook_gates_ready(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (hook->require_security && (!rrc->as_security_activated || !hook->submitted[NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE]))
    return nr_ue_hook_context_result(rrc, "security_gate_not_ready");
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  if (hook->require_reconfiguration_complete
      && (!nb || !nb->hook_meas.valid || nb->hook_meas.completion_submitted_generation != nb->hook_meas.generation))
    return nr_ue_hook_context_result(rrc, "reconfiguration_gate_not_ready");
  return true;
}

static void nr_ue_hook_record_submission(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg)
{
  if (msg > NR_UE_HOOK_MSG_NONE && msg <= NR_UE_HOOK_MSG_MEASUREMENT_REPORT)
    rrc->fuzz_hook.submitted[msg]++;
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  if (msg == NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE && nb && nb->hook_meas.valid)
    nb->hook_meas.completion_submitted_generation = nb->hook_meas.generation;
}

static NR_MeasResults_t *nr_ue_hook_meas_results(NR_MeasurementReport_t *report)
{
  if (!report || report->criticalExtensions.present != NR_MeasurementReport__criticalExtensions_PR_measurementReport
      || !report->criticalExtensions.choice.measurementReport)
    return NULL;
  return &report->criticalExtensions.choice.measurementReport->measResults;
}

/* Restrict seeds to the modeled NR serving-cell shape and quantities. This is
 * not a complete 38.331 validator (neighbor lists and optional extensions are
 * outside this predicate), but prevents caching already-inconsistent seeds. */
static bool nr_ue_hook_report_matches(const NR_MeasResults_t *results, const nr_ue_hook_meas_binding_t *binding)
{
  if (!binding->supported || results->measResultServingMOList.list.count < 1)
    return false;
  for (int i = 0; i < results->measResultServingMOList.list.count; i++) {
    const NR_MeasResultServMO_t *serving = results->measResultServingMOList.list.array[i];
    if (!serving)
      return false;
    const NR_MeasResultNR_t *cell = &serving->measResultServingCell;
    const NR_MeasQuantityResults_t *q = binding->rs_type == NR_NR_RS_Type_ssb ? cell->measResult.cellResults.resultsSSB_Cell
                                                                              : cell->measResult.cellResults.resultsCSI_RS_Cell;
    const NR_MeasQuantityResults_t *other = binding->rs_type == NR_NR_RS_Type_ssb ? cell->measResult.cellResults.resultsCSI_RS_Cell
                                                                                  : cell->measResult.cellResults.resultsSSB_Cell;
    if (!q || other || ((q->rsrp ? 1u : 0u) | (q->rsrq ? 2u : 0u) | (q->sinr ? 4u : 0u)) != binding->quantities)
      return false;
  }
  return true;
}

/* Cache unmodified, encodable reports only. A later generation can reuse them;
 * injected reports must never become the "normal old configuration" seed. */
static void nr_ue_hook_cache_report(NR_UE_RRC_INST_t *rrc, NR_MeasurementReport_t *report)
{
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  NR_MeasResults_t *results = nr_ue_hook_meas_results(report);
  if (!nb || !nb->hook_meas.valid || !results || results->measId < 1 || results->measId > MAX_MEAS_ID)
    return;
  nr_ue_hook_meas_context_t *ctx = &nb->hook_meas;
  if (!nr_ue_hook_report_matches(results, &ctx->binding[results->measId]))
    return;
  nr_ue_hook_report_cache_t *cache = &ctx->current_report;
  asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_MeasurementReport, NULL, report, cache->bytes, sizeof(cache->bytes));
  cache->size = enc.encoded > 0 ? (enc.encoded + 7) / 8 : 0;
  cache->generation = ctx->generation;
  cache->meas_id = results->measId;
  cache->binding = ctx->binding[results->measId];
}

static bool nr_ue_hook_mutate_meas_context(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  const bool id_mode = !strcmp(mode, "configured_measId_mismatch") || !strcmp(mode, "removed_measId_reuse");
  if (strcmp(rrc->fuzz_hook.field_mutation.field, id_mode ? "measId" : "measResults"))
    return nr_ue_hook_context_result(rrc, "context_field_mode_mismatch");
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  NR_MeasResults_t *results = nr_ue_hook_meas_results(payload);
  if (!nb || !nb->hook_meas.valid || !results)
    return nr_ue_hook_context_result(rrc, "measurement_context_missing");
  nr_ue_hook_meas_context_t *ctx = &nb->hook_meas;
  const int original = results->measId;
  if (original < 1 || original > MAX_MEAS_ID || !ctx->binding[original].supported)
    return nr_ue_hook_context_result(rrc, "source_meas_id_not_active");
  const nr_ue_hook_meas_binding_t *binding = &ctx->binding[original];
  if (!nr_ue_hook_source_supported(binding))
    return nr_ue_hook_context_result(rrc, "source_report_quantities_unsupported");
  if (!nr_ue_hook_report_matches(results, binding))
    return nr_ue_hook_context_result(rrc, "source_report_not_consistent");
  if (!strcmp(mode, "configured_measId_mismatch") || !strcmp(mode, "removed_measId_reuse")) {
    int chosen = 0;
    for (int id = 1; id <= MAX_MEAS_ID; id++) {
      if (!ctx->binding[id].configured && (strcmp(mode, "removed_measId_reuse") || ctx->removed[id])) {
        chosen = id;
        break;
      }
    }
    if (!chosen)
      return nr_ue_hook_context_result(rrc, "no_eligible_meas_id");
    results->measId = chosen;
  } else if (!strcmp(mode, "stale_report_after_reconfiguration")) {
    nr_ue_hook_report_cache_t *old = &ctx->previous_report;
    if (!old->size || old->generation >= ctx->generation || old->meas_id < 1 || old->meas_id > MAX_MEAS_ID)
      return nr_ue_hook_context_result(rrc, "no_incompatible_old_report");
    NR_MeasurementReport_t *decoded = NULL;
    asn_dec_rval_t dec = uper_decode(NULL, &asn_DEF_NR_MeasurementReport, (void **)&decoded, old->bytes, old->size, 0, 0);
    if (dec.code != RC_OK) {
      ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, decoded);
      return nr_ue_hook_context_result(rrc, "old_report_decode_failed");
    }
    NR_MeasResults_t *old_results = nr_ue_hook_meas_results(decoded);
    const nr_ue_hook_meas_binding_t *current = &ctx->binding[old->meas_id];
    // A renamed reportConfig/measObject alone proves no violation. Require a
    // removed measId or an actual serving-cell shape/quantity incompatibility.
    if (!old_results || old_results->measId != old->meas_id
        || (current->configured && (!current->supported || nr_ue_hook_report_matches(old_results, current)))) {
      ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, decoded);
      return nr_ue_hook_context_result(rrc, "no_incompatible_old_report");
    }
    ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_MeasurementReport, payload);
    *(NR_MeasurementReport_t *)payload = *decoded;
    free(decoded);
    results = nr_ue_hook_meas_results(payload);
  } else {
    if (results->measResultServingMOList.list.count < 1 || !results->measResultServingMOList.list.array[0])
      return nr_ue_hook_context_result(rrc, "serving_result_missing");
    NR_MeasResultNR_t *cell = &results->measResultServingMOList.list.array[0]->measResultServingCell;
    NR_MeasQuantityResults_t **expected = binding->rs_type == NR_NR_RS_Type_ssb ? &cell->measResult.cellResults.resultsSSB_Cell
                                                                                : &cell->measResult.cellResults.resultsCSI_RS_Cell;
    NR_MeasQuantityResults_t **other = binding->rs_type == NR_NR_RS_Type_ssb ? &cell->measResult.cellResults.resultsCSI_RS_Cell
                                                                             : &cell->measResult.cellResults.resultsSSB_Cell;
    if (!*expected)
      return nr_ue_hook_context_result(rrc, "expected_rs_result_missing");
    if (!strcmp(mode, "rsType_shape_mismatch")) {
      ASN_STRUCT_FREE(asn_DEF_NR_MeasQuantityResults, *other);
      *other = *expected;
      *expected = NULL;
    } else if (!strcmp(mode, "reportQuantity_mismatch")) {
      long **slots[] = {&(*expected)->rsrp, &(*expected)->rsrq, &(*expected)->sinr};
      bool changed = false;
      for (int q = 0; q < 3 && !changed; q++) {
        bool requested = (binding->quantities & (1u << q)) != 0;
        if (requested && *slots[q]) {
          free(*slots[q]);
          *slots[q] = NULL;
          changed = true;
        } else if (!requested && !*slots[q]) {
          *slots[q] = calloc(1, sizeof(long)); // zero is legal for all three ASN.1 ranges
          changed = *slots[q] != NULL;
        }
      }
      if (!changed)
        return nr_ue_hook_context_result(rrc, "quantity_already_inconsistent");
    } else {
      return nr_ue_hook_context_result(rrc, "unsupported_context_mode");
    }
  }
  rrc->fuzz_hook.mutation_generation = ctx->generation;
  rrc->fuzz_hook.original_meas_id = original;
  rrc->fuzz_hook.mutated_meas_id = results->measId;
  nr_ue_hook_context_result(rrc, "applied");
  return true;
}

static void nr_ue_hook_record_enquiry(NR_UE_RRC_INST_t *rrc, NR_UECapabilityEnquiry_t *enquiry)
{
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  if (!nb)
    return;
  nb->hook_requested_rats = 0;
  nb->hook_capability_enquiry_valid = false;
  if (enquiry->criticalExtensions.present != NR_UECapabilityEnquiry__criticalExtensions_PR_ueCapabilityEnquiry
      || !enquiry->criticalExtensions.choice.ueCapabilityEnquiry)
    return;
  NR_UE_CapabilityRAT_RequestList_t *list = &enquiry->criticalExtensions.choice.ueCapabilityEnquiry->ue_CapabilityRAT_RequestList;
  for (int i = 0; i < list->list.count; i++) {
    long rat = list->list.array[i]->rat_Type;
    if (rat >= 0 && rat <= NR_RAT_Type_utra_fdd_v1610)
      nb->hook_requested_rats |= 1u << rat;
  }
  nb->hook_capability_enquiry_valid = true;
}

/* Mutate the outer RAT discriminator only. This intentionally makes the RAT
 * tag disagree with both the enquiry and the unchanged encoded NR container. */
static bool nr_ue_hook_mutate_rat(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  NR_UECapabilityInformation_t *info = payload;
  if (strcmp(mode, "requested_rat_mismatch") || !nb || !nb->hook_capability_enquiry_valid || !nb->hook_requested_rats
      || info->criticalExtensions.present != NR_UECapabilityInformation__criticalExtensions_PR_ueCapabilityInformation
      || !info->criticalExtensions.choice.ueCapabilityInformation)
    return nr_ue_hook_context_result(rrc, "capability_context_missing");
  NR_UE_CapabilityRAT_ContainerList_t *list =
      info->criticalExtensions.choice.ueCapabilityInformation->ue_CapabilityRAT_ContainerList;
  if (!list || list->list.count == 0 || !list->list.array[0])
    return nr_ue_hook_context_result(rrc, "capability_container_missing");
  NR_UE_CapabilityRAT_Container_t *container = list->list.array[0];
  for (int rat = NR_RAT_Type_nr; rat <= NR_RAT_Type_eutra; rat++) {
    if (!(nb->hook_requested_rats & (1u << rat)) && container->rat_Type != rat) {
      container->rat_Type = rat;
      nr_ue_hook_context_result(rrc, "applied");
      return true;
    }
  }
  return nr_ue_hook_context_result(rrc, "no_unrequested_rat");
}

static bool nr_ue_hook_mutate_plmn(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  rrcPerNB_t *nb = nr_ue_hook_context_nb(rrc);
  NR_RRCSetupComplete_t *setup = payload;
  if (strcmp(mode, "configured_plmn_mismatch") || !nb || !nb->SInfo.sib1_validity || nb->hook_plmn_count < 1
      || nb->hook_plmn_count >= 12
      || setup->criticalExtensions.present != NR_RRCSetupComplete__criticalExtensions_PR_rrcSetupComplete
      || !setup->criticalExtensions.choice.rrcSetupComplete)
    return nr_ue_hook_context_result(rrc, "plmn_context_unavailable_or_full");
  long *value = &setup->criticalExtensions.choice.rrcSetupComplete->selectedPLMN_Identity;
  if (*value < 1 || *value > nb->hook_plmn_count)
    return nr_ue_hook_context_result(rrc, "source_plmn_not_configured");
  *value = nb->hook_plmn_count + 1; // valid ASN.1 1..12, absent from the SIB1 list
  nr_ue_hook_context_result(rrc, "applied");
  return true;
}

/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
static void nr_ue_fuzz_hook_bootstrap_measurement_report(NR_UE_RRC_INST_t *rrc, int gnb)
{
  if (!rrc || gnb < 0 || gnb >= NB_CNX_UE)
    return;
  nr_ue_fuzz_hook_reload_config(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (!hook->enabled || hook->target_msg != NR_UE_HOOK_MSG_MEASUREMENT_REPORT || !hook->measurement_bootstrap_enabled
      || hook->measurement_bootstrap_done)
    return;
  hook->context_gnb = gnb;
  if (!nr_ue_hook_gates_ready(rrc)) {
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }
  rrcPerNB_t *nb = &rrc->perNB[gnb];
  int chosen = 0;
  for (int id = 1; nb->hook_meas.valid && id <= MAX_MEAS_ID; id++) {
    if (nr_ue_hook_source_supported(&nb->hook_meas.binding[id])) {
      chosen = id;
      break;
    }
  }
  if (!chosen) {
    nr_ue_hook_context_result(rrc, "bootstrap_source_config_unsupported");
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }
  /* Do not consume or replace the natural event reporting schedule. */
  l3_measurements_t saved = nb->l3_measurements;
  nb->l3_measurements.trigger_to_measid = chosen;
  nb->l3_measurements.trigger_quantity = NR_MeasTriggerQuantityOffset_PR_rsrp;
  nb->l3_measurements.rs_type = nb->hook_meas.binding[chosen].rs_type;
  nb->l3_measurements.neighbor_cell_valid = false;
  unsigned long submitted = hook->submitted[NR_UE_HOOK_MSG_MEASUREMENT_REPORT];
  unsigned long fired = hook->hook_fire_count;
  nr_ue_fuzz_hook_action_t action = hook->action;
  rrc_ue_generate_measurementReport(nb, rrc->ue_id);
  nb->l3_measurements = saved;
  /* Failed gates, mutations and PDCP submissions remain retryable. */
  hook->measurement_bootstrap_done =
      hook->hook_fire_count > fired
      && (action == NR_UE_HOOK_ACTION_DROP || hook->submitted[NR_UE_HOOK_MSG_MEASUREMENT_REPORT] > submitted);
  nr_ue_fuzz_hook_write_state(rrc);
}

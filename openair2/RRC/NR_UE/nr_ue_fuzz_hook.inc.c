/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* UE hook control, state observation and transmission policy. */

static const char *nr_ue_fuzz_hook_msg_name(nr_ue_fuzz_hook_msg_t msg)
{
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE:
      return "RRCSetupComplete";
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE:
      return "SecurityModeComplete";
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE:
      return "RRCReconfigurationComplete";
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE:
      return "RRCReestablishmentComplete";
    case NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION:
      return "UECapabilityInformation";
    case NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER:
      return "ULInformationTransfer";
    case NR_UE_HOOK_MSG_MEASUREMENT_REPORT:
      return "MeasurementReport";
    case NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION:
      return "DL_RRCReconfiguration";
    case NR_UE_HOOK_MSG_DL_SECURITY_MODE_COMMAND:
      return "DL_SecurityModeCommand";
    case NR_UE_HOOK_MSG_DL_UE_CAPABILITY_ENQUIRY:
      return "DL_UECapabilityEnquiry";
    case NR_UE_HOOK_MSG_DL_RRC_REESTABLISHMENT:
      return "DL_RRCReestablishment";
    case NR_UE_HOOK_MSG_DL_RRC_RELEASE:
      return "DL_RRCRelease";
    case NR_UE_HOOK_MSG_NONE:
    default:
      return "NONE";
  }
}

static const char *nr_ue_fuzz_hook_action_name(nr_ue_fuzz_hook_action_t action)
{
  switch (action) {
    case NR_UE_HOOK_ACTION_DROP:
      return "drop";
    case NR_UE_HOOK_ACTION_DUPLICATE:
      return "duplicate";
    case NR_UE_HOOK_ACTION_REPLAY:
      return "replay";
    case NR_UE_HOOK_ACTION_DELAY:
      return "delay";
    case NR_UE_HOOK_ACTION_MUTATE_TXN:
      return "mutate_txn";
    case NR_UE_HOOK_ACTION_MUTATE_FIELD:
      return "mutate_field";
    case NR_UE_HOOK_ACTION_INTEGRITY_FAILURE:
      return "integrity_failure";
    case NR_UE_HOOK_ACTION_NONE:
    default:
      return "none";
  }
}

static const char *nr_ue_fuzz_hook_rrc_state_name(Rrc_State_NR_t state)
{
  switch (state) {
    case RRC_STATE_IDLE_NR:
      return "IDLE";
    case RRC_STATE_INACTIVE_NR:
      return "INACTIVE";
    case RRC_STATE_CONNECTED_NR:
      return "CONNECTED";
    case RRC_STATE_DETACH_NR:
      return "DETACH";
    default:
      return "UNKNOWN";
  }
}

static void nr_ue_fuzz_hook_sleep_ms(int delay_ms);

static nr_ue_fuzz_hook_msg_t nr_ue_fuzz_hook_msg_from_name(const char *name)
{
  if (!name || *name == '\0')
    return NR_UE_HOOK_MSG_NONE;
  if (!strcasecmp(name, "RRCSetupComplete"))
    return NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE;
  if (!strcasecmp(name, "SecurityModeComplete"))
    return NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE;
  if (!strcasecmp(name, "RRCReconfigurationComplete"))
    return NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE;
  if (!strcasecmp(name, "RRCReestablishmentComplete"))
    return NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE;
  if (!strcasecmp(name, "UECapabilityInformation"))
    return NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION;
  if (!strcasecmp(name, "ULInformationTransfer"))
    return NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER;
  if (!strcasecmp(name, "MeasurementReport"))
    return NR_UE_HOOK_MSG_MEASUREMENT_REPORT;
  if (!strcasecmp(name, "DL_RRCReconfiguration") || !strcasecmp(name, "RRCReconfiguration"))
    return NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION;
  if (!strcasecmp(name, "DL_SecurityModeCommand") || !strcasecmp(name, "SecurityModeCommand"))
    return NR_UE_HOOK_MSG_DL_SECURITY_MODE_COMMAND;
  if (!strcasecmp(name, "DL_UECapabilityEnquiry") || !strcasecmp(name, "UECapabilityEnquiry"))
    return NR_UE_HOOK_MSG_DL_UE_CAPABILITY_ENQUIRY;
  if (!strcasecmp(name, "DL_RRCReestablishment") || !strcasecmp(name, "RRCReestablishment"))
    return NR_UE_HOOK_MSG_DL_RRC_REESTABLISHMENT;
  if (!strcasecmp(name, "DL_RRCRelease") || !strcasecmp(name, "RRCRelease"))
    return NR_UE_HOOK_MSG_DL_RRC_RELEASE;
  return NR_UE_HOOK_MSG_NONE;
}

static nr_ue_fuzz_hook_action_t nr_ue_fuzz_hook_action_from_name(const char *name)
{
  if (!name || *name == '\0')
    return NR_UE_HOOK_ACTION_NONE;
  if (!strcasecmp(name, "drop"))
    return NR_UE_HOOK_ACTION_DROP;
  if (!strcasecmp(name, "duplicate"))
    return NR_UE_HOOK_ACTION_DUPLICATE;
  if (!strcasecmp(name, "replay"))
    return NR_UE_HOOK_ACTION_REPLAY;
  if (!strcasecmp(name, "delay"))
    return NR_UE_HOOK_ACTION_DELAY;
  if (!strcasecmp(name, "mutate_txn"))
    return NR_UE_HOOK_ACTION_MUTATE_TXN;
  if (!strcasecmp(name, "mutate_field"))
    return NR_UE_HOOK_ACTION_MUTATE_FIELD;
  if (!strcasecmp(name, "integrity_failure"))
    return NR_UE_HOOK_ACTION_INTEGRITY_FAILURE;
  return NR_UE_HOOK_ACTION_NONE;
}

static void nr_ue_fuzz_hook_copy_text(char *dst, size_t dst_size, const char *src)
{
  if (!dst || dst_size == 0)
    return;
  if (!src)
    src = "";
  snprintf(dst, dst_size, "%s", src);
}

static void nr_ue_fuzz_hook_reset_one_field_mutation(nr_ue_fuzz_hook_field_mutation_t *mutation)
{
  if (!mutation)
    return;
  mutation->enabled = false;
  mutation->message[0] = '\0';
  mutation->adapter_key[0] = '\0';
  mutation->domain_id[0] = '\0';
  mutation->field[0] = '\0';
  mutation->operator_family[0] = '\0';
  mutation->transform_name[0] = '\0';
  mutation->selected_mode[0] = '\0';
  mutation->override_value[0] = '\0';
  mutation->has_range_min = false;
  mutation->range_min = 0;
  mutation->has_range_max = false;
  mutation->range_max = 0;
}

static void nr_ue_fuzz_hook_reset_field_mutation(nr_ue_fuzz_hook_state_t *hook)
{
  nr_ue_fuzz_hook_reset_one_field_mutation(&hook->field_mutation);
  for (unsigned int i = 0; i < NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS; ++i)
    nr_ue_fuzz_hook_reset_one_field_mutation(&hook->field_mutations[i]);
  hook->field_mutation_count = 0;
  hook->field_mutation_applied_count = 0;
  hook->field_mutation_failed_index = 0;
  hook->field_mutation_result[0] = '\0';
}

static unsigned long nr_ue_fuzz_hook_now_ms(const NR_UE_RRC_INST_t *rrc)
{
  if (!rrc)
    return 0;
  const unsigned long hfn = rrc->current_hfn > 0 ? (unsigned long)rrc->current_hfn : 0;
  const unsigned long frame = rrc->current_frame > 0 ? (unsigned long)rrc->current_frame : 0;
  return (hfn * 1024UL + frame) * 10UL;
}

#include "nr_ue_fuzz_hook_context.inc.c"

static void nr_ue_fuzz_hook_write_timer_state(FILE *fp, const char *name, const NR_timer_t *timer)
{
  if (!fp || !name || !timer)
    return;

  const bool active = nr_timer_is_active(timer);
  fprintf(fp, "timer.%s=%s\n", name, active ? "running" : "stopped");
  fprintf(fp, "timer.%s.active=%d\n", name, active ? 1 : 0);
  fprintf(fp, "timer.%s.elapsed_ms=%u\n", name, nr_timer_elapsed_time(timer));
  fprintf(fp, "timer.%s.target_ms=%u\n", name, timer->target);
  fprintf(fp, "timer.%s.remaining_ms=%u\n", name, active ? nr_timer_remaining_time(timer) : 0);
  fprintf(fp, "timer.%s.suspended=%d\n", name, timer->suspended ? 1 : 0);
}

static char *nr_ue_fuzz_hook_trim(char *s)
{
  while (*s && isspace((unsigned char)*s))
    s++;
  size_t len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[len - 1] = '\0';
    len--;
  }
  return s;
}

static void nr_ue_fuzz_hook_ensure_paths(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (hook->control_path[0] != '\0')
    return;
  const char *root = getenv("OAI_NR_UE_HOOK_ROOT");
  if (!root || root[0] == '\0')
    root = "/tmp";
  const size_t root_len = strlen(root);
  const char *separator = root_len > 0 && root[root_len - 1] == '/' ? "" : "/";
  snprintf(hook->control_path, sizeof(hook->control_path), "%s%soai_nr_ue_hook_%ld.ctl", root, separator, rrc->ue_id);
  snprintf(hook->state_path, sizeof(hook->state_path), "%s%soai_nr_ue_hook_%ld.state", root, separator, rrc->ue_id);
  hook->control_mtime = -1;
  hook->control_mtime_nsec = -1;
  hook->last_dl_txn = -1;
  hook->txn_offset = 1;
  hook->delay_ms = 0;
  hook->replay_delay_ms = 0;
  hook->replay_mode[0] = '\0';
  hook->measurement_bootstrap_enabled = false;
  hook->measurement_bootstrap_done = false;
  hook->procedure_trigger_enabled = false;
  hook->procedure_trigger_fired = false;
  hook->procedure_trigger_msg = NR_UE_HOOK_MSG_NONE;
  hook->procedure_trigger_action = NR_UE_HOOK_ACTION_NONE;
  hook->procedure_trigger_delay_ms = 0;
  hook->rrc_reconfiguration_complete_submit_time_valid = false;
  hook->rrc_reconfiguration_complete_submit_time_ms = 0;
  hook->procedure_trigger_waited_ms = 0;
}

static void nr_ue_fuzz_hook_write_state(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_ensure_paths(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  NR_UE_Timers_Constants_t *timers = &rrc->timers_and_constants;
  FILE *fp = fopen(hook->state_path, "w");
  if (!fp)
    return;
  fprintf(fp, "enabled=%d\n", hook->enabled ? 1 : 0);
  fprintf(fp, "target=%s\n", nr_ue_fuzz_hook_msg_name(hook->target_msg));
  fprintf(fp, "action=%s\n", nr_ue_fuzz_hook_action_name(hook->action));
  fprintf(fp, "arm_once=%d\n", hook->arm_once ? 1 : 0);
  fprintf(fp, "txn_offset=%d\n", hook->txn_offset);
  fprintf(fp, "delay_ms=%d\n", hook->delay_ms);
  fprintf(fp, "replay_delay_ms=%d\n", hook->replay_delay_ms);
  fprintf(fp, "replay_mode=%s\n", hook->replay_mode);
  fprintf(fp, "measurement_bootstrap_enabled=%d\n", hook->measurement_bootstrap_enabled ? 1 : 0);
  fprintf(fp, "measurement_bootstrap_done=%d\n", hook->measurement_bootstrap_done ? 1 : 0);
  fprintf(fp, "procedure_trigger_enabled=%d\n", hook->procedure_trigger_enabled ? 1 : 0);
  fprintf(fp, "procedure_trigger_fired=%d\n", hook->procedure_trigger_fired ? 1 : 0);
  fprintf(fp, "procedure_trigger_target=%s\n", nr_ue_fuzz_hook_msg_name(hook->procedure_trigger_msg));
  fprintf(fp, "procedure_trigger_action=%s\n", nr_ue_fuzz_hook_action_name(hook->procedure_trigger_action));
  fprintf(fp, "procedure_trigger_delay_ms=%d\n", hook->procedure_trigger_delay_ms);
  fprintf(fp,
          "rrc_reconfiguration_complete_submit_time_valid=%d\n",
          hook->rrc_reconfiguration_complete_submit_time_valid ? 1 : 0);
  fprintf(fp, "rrc_reconfiguration_complete_submit_time_ms=%lu\n", hook->rrc_reconfiguration_complete_submit_time_ms);
  fprintf(fp, "procedure_trigger_waited_ms=%lu\n", hook->procedure_trigger_waited_ms);
  fprintf(fp, "procedure_trigger_count=%lu\n", hook->procedure_trigger_count);
  fprintf(fp, "last_procedure_trigger_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_procedure_trigger_msg));
  fprintf(fp, "last_procedure_trigger_action=%s\n", nr_ue_fuzz_hook_action_name(hook->last_procedure_trigger_action));
  fprintf(fp, "hook_fire_count=%lu\n", hook->hook_fire_count);
  fprintf(fp, "last_hook_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_hook_msg));
  fprintf(fp, "last_hook_action=%s\n", nr_ue_fuzz_hook_action_name(hook->last_hook_action));
  fprintf(fp, "last_hook_srb=%d\n", hook->last_hook_srb_id);
  fprintf(fp, "nr_rrc_state=%s\n", nr_ue_fuzz_hook_rrc_state_name(rrc->nrRrcState));
  fprintf(fp, "as_security_activated=%d\n", rrc->as_security_activated ? 1 : 0);
  fprintf(fp, "context_gnb=%d\n", hook->context_gnb);
  fprintf(fp, "context_result=%s\n", hook->context_result);
  fprintf(fp, "mutation_generation=%lu\n", hook->mutation_generation);
  fprintf(fp, "original_meas_id=%d\n", hook->original_meas_id);
  fprintf(fp, "mutated_meas_id=%d\n", hook->mutated_meas_id);
  for (int msg = NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE; msg <= NR_UE_HOOK_MSG_MEASUREMENT_REPORT; msg++)
    fprintf(fp, "submitted.%s=%lu\n", nr_ue_fuzz_hook_msg_name(msg), hook->submitted[msg]);
  for (int gnb = 0; gnb < NB_CNX_UE; gnb++) {
    const nr_ue_hook_meas_context_t *ctx = &rrc->perNB[gnb].hook_meas;
    fprintf(fp, "meas.%d.valid=%d\n", gnb, ctx->valid);
    fprintf(fp, "meas.%d.generation=%lu\n", gnb, ctx->generation);
    fprintf(fp, "meas.%d.completion_submitted_generation=%lu\n", gnb, ctx->completion_submitted_generation);
    fprintf(fp, "capability.%d.enquiry_valid=%d\n", gnb, rrc->perNB[gnb].hook_capability_enquiry_valid);
    fprintf(fp, "capability.%d.requested_rats=%u\n", gnb, rrc->perNB[gnb].hook_requested_rats);
    fprintf(fp, "setup.%d.plmn_count=%d\n", gnb, rrc->perNB[gnb].hook_plmn_count);
    for (int id = 1; id <= MAX_MEAS_ID; id++) {
      const nr_ue_hook_meas_binding_t *b = &ctx->binding[id];
      fprintf(fp, "meas.%d.%d.configured=%d\n", gnb, id, b->configured);
      fprintf(fp, "meas.%d.%d.supported=%d\n", gnb, id, b->supported);
      fprintf(fp, "meas.%d.%d.removed=%d\n", gnb, id, ctx->removed[id]);
      if (b->configured) {
        fprintf(fp, "meas.%d.%d.object_id=%d\n", gnb, id, b->object_id);
        fprintf(fp, "meas.%d.%d.report_id=%d\n", gnb, id, b->report_id);
        fprintf(fp, "meas.%d.%d.rs_type=%d\n", gnb, id, b->rs_type);
        fprintf(fp, "meas.%d.%d.quantities=%u\n", gnb, id, b->quantities);
      }
    }
  }
  nr_ue_fuzz_hook_write_timer_state(fp, "T310", &timers->T310);
  nr_ue_fuzz_hook_write_timer_state(fp, "T300", &timers->T300);
  nr_ue_fuzz_hook_write_timer_state(fp, "T301", &timers->T301);
  nr_ue_fuzz_hook_write_timer_state(fp, "T304", &timers->T304);
  nr_ue_fuzz_hook_write_timer_state(fp, "T311", &timers->T311);
  nr_ue_fuzz_hook_write_timer_state(fp, "T319", &timers->T319);
  nr_ue_fuzz_hook_write_timer_state(fp, "T320", &timers->T320);
  nr_ue_fuzz_hook_write_timer_state(fp, "T321", &timers->T321);
  fprintf(fp, "last_dl_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_dl_msg));
  fprintf(fp, "last_dl_txn=%d\n", hook->last_dl_txn);
  fprintf(fp, "seen_rrc_setup_complete=%d\n", hook->seen_rrc_setup_complete ? 1 : 0);
  fprintf(fp, "seen_security_mode_complete=%d\n", hook->seen_security_mode_complete ? 1 : 0);
  fprintf(fp, "seen_rrc_reconfiguration_complete=%d\n", hook->seen_rrc_reconfiguration_complete ? 1 : 0);
  fprintf(fp, "seen_rrc_reestablishment_complete=%d\n", hook->seen_rrc_reestablishment_complete ? 1 : 0);
  fprintf(fp, "seen_ue_capability_information=%d\n", hook->seen_ue_capability_information ? 1 : 0);
  fprintf(fp, "seen_ul_information_transfer=%d\n", hook->seen_ul_information_transfer ? 1 : 0);
  fprintf(fp, "seen_measurement_report=%d\n", hook->seen_measurement_report ? 1 : 0);
  fprintf(fp, "seen_reconfiguration=%d\n", hook->seen_reconfiguration ? 1 : 0);
  fprintf(fp, "seen_security_mode_command=%d\n", hook->seen_security_mode_command ? 1 : 0);
  fprintf(fp, "seen_ue_capability_enquiry=%d\n", hook->seen_ue_capability_enquiry ? 1 : 0);
  fprintf(fp, "seen_rrc_reestablishment=%d\n", hook->seen_rrc_reestablishment ? 1 : 0);
  fprintf(fp, "seen_rrc_release=%d\n", hook->seen_rrc_release ? 1 : 0);
  fprintf(fp, "last_ul_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_ul_msg));
  fprintf(fp, "last_ul_srb=%d\n", hook->last_ul_srb_id);
  fprintf(fp, "last_ul_size=%d\n", hook->last_ul_size);
  fprintf(fp, "field_mutation_enabled=%d\n", hook->field_mutation.enabled ? 1 : 0);
  fprintf(fp, "field_mutation_message=%s\n", hook->field_mutation.message);
  fprintf(fp, "field_mutation_adapter_key=%s\n", hook->field_mutation.adapter_key);
  fprintf(fp, "field_mutation_domain_id=%s\n", hook->field_mutation.domain_id);
  fprintf(fp, "field_mutation_field=%s\n", hook->field_mutation.field);
  fprintf(fp, "field_mutation_operator_family=%s\n", hook->field_mutation.operator_family);
  fprintf(fp, "field_mutation_transform=%s\n", hook->field_mutation.transform_name);
  fprintf(fp, "field_mutation_selected_mode=%s\n", hook->field_mutation.selected_mode);
  fprintf(fp, "field_mutation_value_space_minimum=%d\n", hook->field_mutation.has_range_min ? hook->field_mutation.range_min : 0);
  fprintf(fp, "field_mutation_value_space_maximum=%d\n", hook->field_mutation.has_range_max ? hook->field_mutation.range_max : 0);
  fprintf(fp, "field_mutation_count=%u\n", hook->field_mutation_count);
  fprintf(fp, "field_mutation_applied_count=%u\n", hook->field_mutation_applied_count);
  fprintf(fp, "field_mutation_failed_index=%u\n", hook->field_mutation_failed_index);
  fprintf(fp, "field_mutation_result=%s\n", hook->field_mutation_result);
  for (unsigned int i = 0; i < hook->field_mutation_count && i < NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS; ++i) {
    const nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutations[i];
    fprintf(fp, "field_mutation_%u_enabled=%d\n", i, mutation->enabled ? 1 : 0);
    fprintf(fp, "field_mutation_%u_message=%s\n", i, mutation->message);
    fprintf(fp, "field_mutation_%u_adapter_key=%s\n", i, mutation->adapter_key);
    fprintf(fp, "field_mutation_%u_domain_id=%s\n", i, mutation->domain_id);
    fprintf(fp, "field_mutation_%u_field=%s\n", i, mutation->field);
    fprintf(fp, "field_mutation_%u_operator_family=%s\n", i, mutation->operator_family);
    fprintf(fp, "field_mutation_%u_transform=%s\n", i, mutation->transform_name);
    fprintf(fp, "field_mutation_%u_selected_mode=%s\n", i, mutation->selected_mode);
    fprintf(fp,
            "field_mutation_%u_value_space_minimum=%d\n",
            i,
            mutation->has_range_min ? mutation->range_min : 0);
    fprintf(fp,
            "field_mutation_%u_value_space_maximum=%d\n",
            i,
            mutation->has_range_max ? mutation->range_max : 0);
  }
  fclose(fp);
}

static void nr_ue_fuzz_hook_disarm(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  hook->enabled = false;
  hook->arm_once = false;
  hook->target_msg = NR_UE_HOOK_MSG_NONE;
  hook->action = NR_UE_HOOK_ACTION_NONE;
  hook->txn_offset = 1;
  hook->delay_ms = 0;
  hook->replay_delay_ms = 0;
  hook->replay_mode[0] = '\0';
  hook->measurement_bootstrap_enabled = false;
  hook->measurement_bootstrap_done = false;
  hook->require_security = false;
  hook->require_reconfiguration_complete = false;
  hook->procedure_trigger_enabled = false;
  hook->procedure_trigger_fired = false;
  hook->procedure_trigger_msg = NR_UE_HOOK_MSG_NONE;
  hook->procedure_trigger_action = NR_UE_HOOK_ACTION_NONE;
  hook->procedure_trigger_delay_ms = 0;
  hook->rrc_reconfiguration_complete_submit_time_valid = false;
  hook->rrc_reconfiguration_complete_submit_time_ms = 0;
  hook->procedure_trigger_waited_ms = 0;
  nr_ue_fuzz_hook_reset_field_mutation(hook);
}

static void nr_ue_fuzz_hook_disarm_persistent(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_ensure_paths(rrc);
  nr_ue_fuzz_hook_disarm(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (unlink(hook->control_path) != 0 && errno != ENOENT)
    LOG_W(NR_RRC, "[UE %ld][HOOK] failed to remove ctl %s: %s\n", rrc->ue_id, hook->control_path, strerror(errno));
  hook->control_mtime = -1;
  hook->control_mtime_nsec = -1;
}

static void nr_ue_fuzz_hook_record_fire(NR_UE_RRC_INST_t *rrc,
                                        nr_ue_fuzz_hook_msg_t msg,
                                        nr_ue_fuzz_hook_action_t action,
                                        int srb_id)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  hook->hook_fire_count++;
  hook->last_hook_msg = msg;
  hook->last_hook_action = action;
  hook->last_hook_srb_id = srb_id;
}

static bool nr_ue_fuzz_hook_parse_field_mutation_key(nr_ue_fuzz_hook_field_mutation_t *mutation,
                                                     const char *key,
                                                     const char *value)
{
  if (!mutation || !key || !value)
    return false;
  if (!strcasecmp(key, "enabled"))
    mutation->enabled = atoi(value) != 0;
  else if (!strcasecmp(key, "message"))
    nr_ue_fuzz_hook_copy_text(mutation->message, sizeof(mutation->message), value);
  else if (!strcasecmp(key, "adapter_key"))
    nr_ue_fuzz_hook_copy_text(mutation->adapter_key, sizeof(mutation->adapter_key), value);
  else if (!strcasecmp(key, "domain_id"))
    nr_ue_fuzz_hook_copy_text(mutation->domain_id, sizeof(mutation->domain_id), value);
  else if (!strcasecmp(key, "field"))
    nr_ue_fuzz_hook_copy_text(mutation->field, sizeof(mutation->field), value);
  else if (!strcasecmp(key, "operator_family"))
    nr_ue_fuzz_hook_copy_text(mutation->operator_family, sizeof(mutation->operator_family), value);
  else if (!strcasecmp(key, "transform"))
    nr_ue_fuzz_hook_copy_text(mutation->transform_name, sizeof(mutation->transform_name), value);
  else if (!strcasecmp(key, "selected_mode"))
    nr_ue_fuzz_hook_copy_text(mutation->selected_mode, sizeof(mutation->selected_mode), value);
  else if (!strcasecmp(key, "override_value"))
    nr_ue_fuzz_hook_copy_text(mutation->override_value, sizeof(mutation->override_value), value);
  else if (!strcasecmp(key, "value_space_minimum")) {
    mutation->range_min = atoi(value);
    mutation->has_range_min = true;
  } else if (!strcasecmp(key, "value_space_maximum")) {
    mutation->range_max = atoi(value);
    mutation->has_range_max = true;
  } else {
    return false;
  }
  return true;
}

static void nr_ue_fuzz_hook_reload_config(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_ensure_paths(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  struct stat st = {0};
  if (stat(hook->control_path, &st) != 0 || (hook->control_mtime == st.st_mtime && hook->control_mtime_nsec == st.st_mtim.tv_nsec))
    return;

  hook->control_mtime = st.st_mtime;
  hook->control_mtime_nsec = st.st_mtim.tv_nsec;
  hook->enabled = false;
  hook->arm_once = false;
  hook->target_msg = NR_UE_HOOK_MSG_NONE;
  hook->action = NR_UE_HOOK_ACTION_NONE;
  hook->txn_offset = 1;
  hook->delay_ms = 0;
  hook->replay_delay_ms = 0;
  hook->replay_mode[0] = '\0';
  hook->measurement_bootstrap_enabled = false;
  hook->measurement_bootstrap_done = false;
  hook->require_security = false;
  hook->require_reconfiguration_complete = false;
  hook->procedure_trigger_enabled = false;
  hook->procedure_trigger_fired = false;
  hook->procedure_trigger_msg = NR_UE_HOOK_MSG_NONE;
  hook->procedure_trigger_action = NR_UE_HOOK_ACTION_NONE;
  hook->procedure_trigger_delay_ms = 0;
  hook->rrc_reconfiguration_complete_submit_time_valid = false;
  hook->rrc_reconfiguration_complete_submit_time_ms = 0;
  hook->procedure_trigger_waited_ms = 0;
  hook->context_result[0] = '\0';
  nr_ue_fuzz_hook_reset_field_mutation(hook);

  FILE *fp = fopen(hook->control_path, "r");
  if (!fp)
    return;

  char line[1024];
  while (fgets(line, sizeof(line), fp) != NULL) {
    char *trimmed = nr_ue_fuzz_hook_trim(line);
    if (*trimmed == '\0' || *trimmed == '#')
      continue;
    char *eq = strchr(trimmed, '=');
    if (!eq)
      continue;
    *eq = '\0';
    char *key = nr_ue_fuzz_hook_trim(trimmed);
    char *value = nr_ue_fuzz_hook_trim(eq + 1);

    if (!strcasecmp(key, "enabled"))
      hook->enabled = atoi(value) != 0;
    else if (!strcasecmp(key, "target"))
      hook->target_msg = nr_ue_fuzz_hook_msg_from_name(value);
    else if (!strcasecmp(key, "action"))
      hook->action = nr_ue_fuzz_hook_action_from_name(value);
    else if (!strcasecmp(key, "arm_once"))
      hook->arm_once = atoi(value) != 0;
    else if (!strcasecmp(key, "txn_offset"))
      hook->txn_offset = atoi(value);
    else if (!strcasecmp(key, "delay_ms"))
      hook->delay_ms = atoi(value);
    else if (!strcasecmp(key, "replay_delay_ms"))
      hook->replay_delay_ms = atoi(value);
    else if (!strcasecmp(key, "replay_mode"))
      nr_ue_fuzz_hook_copy_text(hook->replay_mode, sizeof(hook->replay_mode), value);
    else if (!strcasecmp(key, "measurement_bootstrap") || !strcasecmp(key, "measurement_bootstrap_enabled"))
      hook->measurement_bootstrap_enabled = atoi(value) != 0;
    else if (!strcasecmp(key, "procedure_trigger_enabled"))
      hook->procedure_trigger_enabled = atoi(value) != 0;
    else if (!strcasecmp(key, "procedure_trigger_target") || !strcasecmp(key, "procedure_trigger_message"))
      hook->procedure_trigger_msg = nr_ue_fuzz_hook_msg_from_name(value);
    else if (!strcasecmp(key, "procedure_trigger_action"))
      hook->procedure_trigger_action = nr_ue_fuzz_hook_action_from_name(value);
    else if (!strcasecmp(key, "procedure_trigger_delay_ms") || !strcasecmp(key, "procedure_trigger_settle_ms"))
      hook->procedure_trigger_delay_ms = atoi(value);
    else if (!strcasecmp(key, "require_security"))
      hook->require_security = atoi(value) != 0;
    else if (!strcasecmp(key, "require_reconfiguration_complete"))
      hook->require_reconfiguration_complete = atoi(value) != 0;
    else if (!strcasecmp(key, "field_mutation_count")) {
      int count = atoi(value);
      if (count < 0)
        count = 0;
      if (count > NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS)
        count = NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS;
      hook->field_mutation_count = (unsigned int)count;
    } else if (!strncasecmp(key, "field_mutation_", strlen("field_mutation_"))) {
      unsigned int index = 0;
      char subkey[128] = {0};
      if (sscanf(key, "field_mutation_%u_%127s", &index, subkey) == 2 && index < NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS) {
        if (nr_ue_fuzz_hook_parse_field_mutation_key(&hook->field_mutations[index], subkey, value)
            && hook->field_mutation_count <= index)
          hook->field_mutation_count = index + 1;
      } else {
        const char *legacy_key = key + strlen("field_mutation_");
        nr_ue_fuzz_hook_parse_field_mutation_key(&hook->field_mutation, legacy_key, value);
      }
    }
  }
  fclose(fp);

  LOG_I(NR_RRC,
        "[UE %ld][HOOK] loaded ctl enabled=%d target=%s action=%s arm_once=%d txn_offset=%d delay_ms=%d replay_delay_ms=%d "
        "replay_mode=%s measurement_bootstrap=%d bootstrap_done=%d procedure_trigger=%s/%s trigger_delay_ms=%d "
        "field_mutation_count=%u adapter_key=%s operator_family=%s "
        "transform=%s field=%s mode=%s range=[%d,%d]\n",
        rrc->ue_id,
        hook->enabled ? 1 : 0,
        nr_ue_fuzz_hook_msg_name(hook->target_msg),
        nr_ue_fuzz_hook_action_name(hook->action),
        hook->arm_once ? 1 : 0,
        hook->txn_offset,
        hook->delay_ms,
        hook->replay_delay_ms,
        hook->replay_mode,
        hook->measurement_bootstrap_enabled ? 1 : 0,
        hook->measurement_bootstrap_done ? 1 : 0,
        nr_ue_fuzz_hook_msg_name(hook->procedure_trigger_msg),
        nr_ue_fuzz_hook_action_name(hook->procedure_trigger_action),
        hook->procedure_trigger_delay_ms,
        hook->field_mutation_count,
        hook->field_mutation.adapter_key,
        hook->field_mutation.operator_family,
        hook->field_mutation.transform_name,
        hook->field_mutation.field,
        hook->field_mutation.selected_mode,
        hook->field_mutation.has_range_min ? hook->field_mutation.range_min : 0,
        hook->field_mutation.has_range_max ? hook->field_mutation.range_max : 0);
  nr_ue_fuzz_hook_write_state(rrc);
}

static bool nr_ue_fuzz_hook_has_reestablishment_context(const NR_UE_RRC_INST_t *rrc)
{
  if (!rrc->as_security_activated || rrc->nrRrcState != RRC_STATE_CONNECTED_NR)
    return false;
  if (rrc->Srb[2] == RB_NOT_PRESENT)
    return false;
  for (int i = 0; i < MAX_DRBS_PER_UE; i++) {
    if (rrc->status_DRBs[i] != RB_NOT_PRESENT)
      return true;
  }
  return false;
}

static bool nr_ue_fuzz_hook_procedure_trigger_delay_elapsed(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (hook->procedure_trigger_delay_ms <= 0) {
    hook->procedure_trigger_waited_ms = 0;
    return true;
  }

  const unsigned long now_ms = nr_ue_fuzz_hook_now_ms(rrc);
  if (!hook->rrc_reconfiguration_complete_submit_time_valid) {
    hook->rrc_reconfiguration_complete_submit_time_valid = true;
    hook->rrc_reconfiguration_complete_submit_time_ms = now_ms;
    hook->procedure_trigger_waited_ms = 0;
  } else if (now_ms >= hook->rrc_reconfiguration_complete_submit_time_ms) {
    hook->procedure_trigger_waited_ms = now_ms - hook->rrc_reconfiguration_complete_submit_time_ms;
  } else {
    hook->procedure_trigger_waited_ms = 0;
  }

  if (hook->procedure_trigger_waited_ms < (unsigned long)hook->procedure_trigger_delay_ms) {
    nr_ue_hook_context_result(rrc, "integrity_failure_waiting_for_reconfiguration_settle");
    nr_ue_fuzz_hook_write_state(rrc);
    return false;
  }
  return true;
}

static bool nr_ue_fuzz_hook_maybe_inject_integrity_failure(NR_UE_RRC_INST_t *rrc)
{
  nr_ue_fuzz_hook_reload_config(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (!hook->enabled || !hook->procedure_trigger_enabled || hook->procedure_trigger_fired)
    return false;
  if (hook->procedure_trigger_msg == NR_UE_HOOK_MSG_NONE)
    hook->procedure_trigger_msg = NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION;
  if (hook->procedure_trigger_msg != NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION
      || hook->procedure_trigger_action != NR_UE_HOOK_ACTION_INTEGRITY_FAILURE) {
    nr_ue_hook_context_result(rrc, "unsupported_procedure_trigger");
    nr_ue_fuzz_hook_write_state(rrc);
    return false;
  }
  if (hook->submitted[NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE] == 0 || !nr_ue_fuzz_hook_has_reestablishment_context(rrc)) {
    nr_ue_hook_context_result(rrc, "integrity_failure_waiting_for_connected_context");
    nr_ue_fuzz_hook_write_state(rrc);
    return false;
  }
  if (!nr_ue_fuzz_hook_procedure_trigger_delay_elapsed(rrc))
    return false;

  LOG_W(NR_RRC,
        "[UE %ld][HOOK] inject integrity failure after %s to trigger RRC reestablishment (waited %lu ms)\n",
        rrc->ue_id,
        nr_ue_fuzz_hook_msg_name(hook->procedure_trigger_msg),
        hook->procedure_trigger_waited_ms);
  hook->procedure_trigger_fired = true;
  hook->procedure_trigger_count++;
  hook->last_procedure_trigger_msg = hook->procedure_trigger_msg;
  hook->last_procedure_trigger_action = hook->procedure_trigger_action;
  nr_ue_fuzz_hook_record_fire(rrc, hook->procedure_trigger_msg, hook->procedure_trigger_action, 1);
  nr_ue_hook_context_result(rrc, "integrity_failure_injected");
  nr_ue_fuzz_hook_write_state(rrc);
  handle_rlf_detection(rrc);
  nr_ue_fuzz_hook_write_state(rrc);
  return true;
}

static void nr_ue_fuzz_hook_record_dl(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, int txn)
{
  nr_ue_fuzz_hook_ensure_paths(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  hook->last_dl_msg = msg;
  hook->last_dl_txn = txn;
  if (msg == NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION)
    hook->seen_reconfiguration = true;
  if (msg == NR_UE_HOOK_MSG_DL_SECURITY_MODE_COMMAND)
    hook->seen_security_mode_command = true;
  if (msg == NR_UE_HOOK_MSG_DL_UE_CAPABILITY_ENQUIRY)
    hook->seen_ue_capability_enquiry = true;
  if (msg == NR_UE_HOOK_MSG_DL_RRC_REESTABLISHMENT)
    hook->seen_rrc_reestablishment = true;
  if (msg == NR_UE_HOOK_MSG_DL_RRC_RELEASE)
    hook->seen_rrc_release = true;
  nr_ue_fuzz_hook_write_state(rrc);
}

static void nr_ue_fuzz_hook_record_seen_ul(nr_ue_fuzz_hook_state_t *hook, nr_ue_fuzz_hook_msg_t msg)
{
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE:
      hook->seen_rrc_setup_complete = true;
      break;
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE:
      hook->seen_security_mode_complete = true;
      break;
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE:
      hook->seen_rrc_reconfiguration_complete = true;
      break;
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE:
      hook->seen_rrc_reestablishment_complete = true;
      break;
    case NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION:
      hook->seen_ue_capability_information = true;
      break;
    case NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER:
      hook->seen_ul_information_transfer = true;
      break;
    case NR_UE_HOOK_MSG_MEASUREMENT_REPORT:
      hook->seen_measurement_report = true;
      break;
    default:
      break;
  }
}

static bool nr_ue_fuzz_hook_preprocess_dl(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, int txn, bool replayed)
{
  nr_ue_fuzz_hook_reload_config(rrc);
  nr_ue_fuzz_hook_record_dl(rrc, msg, txn);

  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (replayed) {
    return false;
  }

  const bool hit_target = hook->enabled && hook->target_msg == msg && nr_ue_hook_gates_ready(rrc);
  if (!hit_target)
    return false;

  if (hook->action == NR_UE_HOOK_ACTION_DROP) {
    LOG_W(NR_RRC, "[UE %ld][HOOK] drop %s on DL-DCCH\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg));
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DROP, -1);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return true;
  }

  if (hook->action == NR_UE_HOOK_ACTION_DELAY) {
    const int delay_ms = hook->delay_ms > 0 ? hook->delay_ms : 50;
    LOG_W(NR_RRC, "[UE %ld][HOOK] delay %s on DL-DCCH by %d ms\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg), delay_ms);
    nr_ue_fuzz_hook_sleep_ms(delay_ms);
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DELAY, -1);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
  }

  return false;
}

static bool nr_ue_fuzz_hook_should_repeat_dl(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, bool replayed)
{
  // A locally replayed PDU must never schedule another replay, even if rearmed.
  if (replayed)
    return false;
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  const bool hit_target = hook->enabled && hook->target_msg == msg && nr_ue_hook_gates_ready(rrc);
  if (!hit_target)
    return false;

  if (hook->action == NR_UE_HOOK_ACTION_DUPLICATE) {
    LOG_W(NR_RRC, "[UE %ld][HOOK] duplicate %s on DL-DCCH\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg));
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DUPLICATE, -1);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return true;
  }

  if (hook->action == NR_UE_HOOK_ACTION_REPLAY) {
    const int replay_delay_ms = hook->replay_delay_ms > 0 ? hook->replay_delay_ms : 0;
    const char *replay_mode = hook->replay_mode[0] != '\0' ? hook->replay_mode : "immediate_replay";
    LOG_W(NR_RRC,
          "[UE %ld][HOOK] replay %s on DL-DCCH mode=%s delay_ms=%d\n",
          rrc->ue_id,
          nr_ue_fuzz_hook_msg_name(msg),
          replay_mode,
          replay_delay_ms);
    nr_ue_fuzz_hook_sleep_ms(replay_delay_ms);
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_REPLAY, -1);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return true;
  }

  return false;
}

static void nr_ue_fuzz_hook_cache_ul(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, int srb_id, const uint8_t *buffer, int size)
{
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  hook->last_ul_msg = msg;
  hook->last_ul_srb_id = srb_id;
  hook->last_ul_size = size > NR_RRC_BUF_SIZE ? NR_RRC_BUF_SIZE : size;
  nr_ue_fuzz_hook_record_seen_ul(hook, msg);
  if (hook->last_ul_size > 0)
    memcpy(hook->last_ul_pdu, buffer, hook->last_ul_size);
  nr_ue_fuzz_hook_write_state(rrc);
}

static void nr_ue_fuzz_hook_sleep_ms(int delay_ms)
{
  if (delay_ms <= 0)
    return;
  usleep((useconds_t)delay_ms * 1000);
}

#include "nr_ue_fuzz_hook_adapters.inc.c"

static void nr_ue_fuzz_hook_send_srb(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, int srb_id, uint8_t *buffer, int size)
{
  nr_ue_fuzz_hook_reload_config(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  const bool hit_target = hook->enabled && hook->target_msg == msg && nr_ue_hook_gates_ready(rrc);

  if (hit_target && hook->action == NR_UE_HOOK_ACTION_DROP) {
    LOG_W(NR_RRC, "[UE %ld][HOOK] drop %s on SRB%d\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id);
    nr_ue_fuzz_hook_cache_ul(rrc, msg, srb_id, buffer, size);
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DROP, srb_id);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }

  if (hit_target && hook->action == NR_UE_HOOK_ACTION_DELAY) {
    const int delay_ms = hook->delay_ms > 0 ? hook->delay_ms : 50;
    LOG_W(NR_RRC, "[UE %ld][HOOK] delay %s on SRB%d by %d ms\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id, delay_ms);
    nr_ue_fuzz_hook_sleep_ms(delay_ms);
  }

  if (!nr_pdcp_data_req_srb(rrc->ue_id, srb_id, 0, size, buffer, deliver_pdu_srb_rlc, NULL)) {
    nr_ue_hook_context_result(rrc, "pdcp_submission_failed");
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }
  nr_ue_rrc_trace_signal(rrc, "TX", nr_ue_fuzz_hook_msg_name(msg), srb_id);
  nr_ue_hook_record_submission(rrc, msg);
  nr_ue_fuzz_hook_cache_ul(rrc, msg, srb_id, buffer, size);

  if (hit_target && hook->action == NR_UE_HOOK_ACTION_DELAY) {
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DELAY, srb_id);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }

  if (hit_target && hook->action == NR_UE_HOOK_ACTION_DUPLICATE) {
    LOG_W(NR_RRC, "[UE %ld][HOOK] duplicate %s on SRB%d\n", rrc->ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id);
    if (!nr_pdcp_data_req_srb(rrc->ue_id, srb_id, 0, size, buffer, deliver_pdu_srb_rlc, NULL)) {
      nr_ue_hook_context_result(rrc, "pdcp_submission_failed");
      nr_ue_fuzz_hook_write_state(rrc);
      return;
    }
    nr_ue_rrc_trace_signal(rrc, "TX", nr_ue_fuzz_hook_msg_name(msg), srb_id);
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_DUPLICATE, srb_id);
    nr_ue_hook_record_submission(rrc, msg);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
    return;
  }

  if (hit_target && hook->action == NR_UE_HOOK_ACTION_REPLAY) {
    const int replay_delay_ms = hook->replay_delay_ms > 0 ? hook->replay_delay_ms : 0;
    const char *replay_mode = hook->replay_mode[0] != '\0' ? hook->replay_mode : "immediate_replay";
    LOG_W(NR_RRC,
          "[UE %ld][HOOK] replay %s on SRB%d mode=%s delay_ms=%d\n",
          rrc->ue_id,
          nr_ue_fuzz_hook_msg_name(msg),
          srb_id,
          replay_mode,
          replay_delay_ms);
    nr_ue_fuzz_hook_sleep_ms(replay_delay_ms);
    if (!nr_pdcp_data_req_srb(rrc->ue_id, srb_id, 0, hook->last_ul_size, hook->last_ul_pdu, deliver_pdu_srb_rlc, NULL)) {
      nr_ue_hook_context_result(rrc, "pdcp_submission_failed");
      nr_ue_fuzz_hook_write_state(rrc);
      return;
    }
    nr_ue_rrc_trace_signal(rrc, "TX", nr_ue_fuzz_hook_msg_name(msg), srb_id);
    nr_ue_fuzz_hook_record_fire(rrc, msg, NR_UE_HOOK_ACTION_REPLAY, srb_id);
    nr_ue_hook_record_submission(rrc, msg);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(rrc);
    nr_ue_fuzz_hook_write_state(rrc);
  }
  nr_ue_fuzz_hook_write_state(rrc);
}

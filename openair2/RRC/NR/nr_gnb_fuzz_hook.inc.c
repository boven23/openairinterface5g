/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* gNB downlink hook control, state observation and post-encode mutation. */

typedef gNB_RRC_UE_t NR_UE_RRC_INST_t;

typedef bool (*nr_ue_fuzz_hook_field_adapter_fn_t)(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode);

typedef struct nr_ue_fuzz_hook_field_adapter_s {
  nr_ue_fuzz_hook_msg_t target_msg;
  const char *adapter_key;
  const char *domain_id;
  const char *message_name;
  const char *field_name;
  const char *operator_family;
  nr_ue_fuzz_hook_field_adapter_fn_t apply;
} nr_ue_fuzz_hook_field_adapter_t;

static const char *nr_ue_fuzz_hook_msg_name(nr_ue_fuzz_hook_msg_t msg)
{
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION:
      return "RRCReconfiguration";
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMMAND:
      return "SecurityModeCommand";
    case NR_UE_HOOK_MSG_UE_CAPABILITY_ENQUIRY:
      return "UECapabilityEnquiry";
    case NR_UE_HOOK_MSG_DL_INFORMATION_TRANSFER:
      return "DLInformationTransfer";
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT:
      return "RRCReestablishment";
    case NR_UE_HOOK_MSG_RRC_RELEASE:
      return "RRCRelease";
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
    case NR_UE_HOOK_ACTION_NONE:
    default:
      return "none";
  }
}

static bool nr_ue_fuzz_hook_text_eq(const char *lhs, const char *rhs)
{
  if (!lhs || !rhs)
    return false;
  return !strcasecmp(lhs, rhs);
}

static nr_ue_fuzz_hook_msg_t nr_ue_fuzz_hook_msg_from_name(const char *name)
{
  if (!name || *name == '\0')
    return NR_UE_HOOK_MSG_NONE;
  if (!strcasecmp(name, "RRCReconfiguration") || !strcasecmp(name, "DL_RRCReconfiguration"))
    return NR_UE_HOOK_MSG_RRC_RECONFIGURATION;
  if (!strcasecmp(name, "SecurityModeCommand") || !strcasecmp(name, "DL_SecurityModeCommand"))
    return NR_UE_HOOK_MSG_SECURITY_MODE_COMMAND;
  if (!strcasecmp(name, "UECapabilityEnquiry") || !strcasecmp(name, "DL_UECapabilityEnquiry"))
    return NR_UE_HOOK_MSG_UE_CAPABILITY_ENQUIRY;
  if (!strcasecmp(name, "DLInformationTransfer") || !strcasecmp(name, "DL_DLInformationTransfer"))
    return NR_UE_HOOK_MSG_DL_INFORMATION_TRANSFER;
  if (!strcasecmp(name, "RRCReestablishment") || !strcasecmp(name, "DL_RRCReestablishment"))
    return NR_UE_HOOK_MSG_RRC_REESTABLISHMENT;
  if (!strcasecmp(name, "RRCRelease") || !strcasecmp(name, "DL_RRCRelease"))
    return NR_UE_HOOK_MSG_RRC_RELEASE;
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
  memset(mutation, 0, sizeof(*mutation));
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

static long nr_gnb_fuzz_hook_external_id(const gNB_RRC_UE_t *ue)
{
  if (!ue || ue->rrc_ue_id == 0)
    return 0;
  return (long)ue->rrc_ue_id - 1;
}

static void nr_gnb_fuzz_hook_ensure_paths(gNB_RRC_UE_t *ue)
{
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  if (hook->control_path[0] != '\0')
    return;
  const char *root = getenv("OAI_NR_GNB_HOOK_ROOT");
  if (!root || root[0] == '\0')
    root = "/tmp";
  const size_t root_len = strlen(root);
  const char *separator = root_len > 0 && root[root_len - 1] == '/' ? "" : "/";
  const long hook_id = nr_gnb_fuzz_hook_external_id(ue);
  snprintf(hook->control_path, sizeof(hook->control_path), "%s%soai_nr_gnb_hook_%ld.ctl", root, separator, hook_id);
  snprintf(hook->state_path, sizeof(hook->state_path), "%s%soai_nr_gnb_hook_%ld.state", root, separator, hook_id);
  hook->control_mtime = -1;
  hook->control_mtime_nsec = -1;
  hook->txn_offset = 1;
  hook->last_dl_txn = -1;
}

static void nr_ue_fuzz_hook_write_state(NR_UE_RRC_INST_t *ue)
{
  nr_gnb_fuzz_hook_ensure_paths(ue);
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
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
  fprintf(fp, "hook_fire_count=%lu\n", hook->hook_fire_count);
  fprintf(fp, "procedure_trigger_count=%lu\n", hook->procedure_trigger_count);
  fprintf(fp, "last_hook_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_hook_msg));
  fprintf(fp, "last_hook_action=%s\n", nr_ue_fuzz_hook_action_name(hook->last_hook_action));
  fprintf(fp, "last_hook_srb=%d\n", hook->last_hook_srb_id);
  fprintf(fp, "last_dl_msg=%s\n", nr_ue_fuzz_hook_msg_name(hook->last_dl_msg));
  fprintf(fp, "last_dl_txn=%d\n", hook->last_dl_txn);
  fprintf(fp, "last_dl_size=%d\n", hook->last_dl_size);
  for (int msg = NR_UE_HOOK_MSG_RRC_RECONFIGURATION; msg <= NR_UE_HOOK_MSG_RRC_RELEASE; msg++)
    fprintf(fp, "submitted.%s=%lu\n", nr_ue_fuzz_hook_msg_name(msg), hook->submitted[msg]);
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
    fprintf(fp, "field_mutation_%u_value_space_minimum=%d\n", i, mutation->has_range_min ? mutation->range_min : 0);
    fprintf(fp, "field_mutation_%u_value_space_maximum=%d\n", i, mutation->has_range_max ? mutation->range_max : 0);
  }
  fclose(fp);
}

static void nr_ue_fuzz_hook_disarm(NR_UE_RRC_INST_t *ue)
{
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  hook->enabled = false;
  hook->arm_once = false;
  hook->target_msg = NR_UE_HOOK_MSG_NONE;
  hook->action = NR_UE_HOOK_ACTION_NONE;
  hook->txn_offset = 1;
  hook->delay_ms = 0;
  hook->replay_delay_ms = 0;
  nr_ue_fuzz_hook_reset_field_mutation(hook);
}

static void nr_ue_fuzz_hook_disarm_persistent(NR_UE_RRC_INST_t *ue)
{
  nr_gnb_fuzz_hook_ensure_paths(ue);
  nr_ue_fuzz_hook_disarm(ue);
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  if (unlink(hook->control_path) != 0 && errno != ENOENT)
    LOG_W(NR_RRC, "[gNB][HOOK] UE %u failed to remove ctl %s: %s\n", ue->rrc_ue_id, hook->control_path, strerror(errno));
  hook->control_mtime = -1;
  hook->control_mtime_nsec = -1;
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

static void nr_ue_fuzz_hook_reload_config(NR_UE_RRC_INST_t *ue)
{
  nr_gnb_fuzz_hook_ensure_paths(ue);
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
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
        "[gNB][HOOK] UE %u loaded ctl enabled=%d target=%s action=%s arm_once=%d field_mutation_count=%u\n",
        ue->rrc_ue_id,
        hook->enabled ? 1 : 0,
        nr_ue_fuzz_hook_msg_name(hook->target_msg),
        nr_ue_fuzz_hook_action_name(hook->action),
        hook->arm_once ? 1 : 0,
        hook->field_mutation_count);
  nr_ue_fuzz_hook_write_state(ue);
}

static void nr_ue_fuzz_hook_record_fire(NR_UE_RRC_INST_t *ue,
                                        nr_ue_fuzz_hook_msg_t msg,
                                        nr_ue_fuzz_hook_action_t action,
                                        int srb_id)
{
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  hook->hook_fire_count++;
  hook->last_hook_msg = msg;
  hook->last_hook_action = action;
  hook->last_hook_srb_id = srb_id;
}

static void nr_gnb_fuzz_hook_record_dl(NR_UE_RRC_INST_t *ue,
                                       nr_ue_fuzz_hook_msg_t msg,
                                       int txn,
                                       const uint8_t *buffer,
                                       int size)
{
  nr_gnb_fuzz_hook_ensure_paths(ue);
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  hook->last_dl_msg = msg;
  hook->last_dl_txn = txn;
  hook->last_dl_size = size > NR_RRC_BUF_SIZE ? NR_RRC_BUF_SIZE : size;
  if (hook->last_dl_size > 0 && buffer)
    memcpy(hook->last_dl_pdu, buffer, hook->last_dl_size);
  if (msg > NR_UE_HOOK_MSG_NONE && msg <= NR_UE_HOOK_MSG_RRC_RELEASE)
    hook->submitted[msg]++;
  nr_ue_fuzz_hook_write_state(ue);
}

static void nr_ue_fuzz_hook_clear_integer_encode_patches(NR_UE_RRC_INST_t *ue)
{
  if (!ue)
    return;
  ue->fuzz_hook.integer_encode_patch_count = 0;
  memset(ue->fuzz_hook.integer_encode_patches, 0, sizeof(ue->fuzz_hook.integer_encode_patches));
}

static bool nr_ue_fuzz_hook_register_integer_encode_patch(NR_UE_RRC_INST_t *ue,
                                                          long *target,
                                                          long requested_value,
                                                          long min_value,
                                                          long max_value)
{
  (void)ue;
  (void)target;
  return requested_value >= min_value && requested_value <= max_value;
}

static void nr_ue_rrc_trace_adapter(NR_UE_RRC_INST_t *ue,
                                    const char *message,
                                    const char *field,
                                    const char *operator_family,
                                    const char *mode,
                                    const char *result)
{
  LOG_I(NR_RRC,
        "[gNB][HOOK] UE %u adapter message=%s field=%s operator=%s mode=%s result=%s\n",
        ue ? ue->rrc_ue_id : 0,
        message ? message : "",
        field ? field : "",
        operator_family ? operator_family : "",
        mode ? mode : "",
        result ? result : "");
}

static bool nr_ue_fuzz_hook_apply_integer_value(NR_UE_RRC_INST_t *ue,
                                                long *value,
                                                const char *mode,
                                                int default_min,
                                                int default_max,
                                                bool allow_runtime_echo)
{
  (void)allow_runtime_echo;
  const nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  const nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutation;
  const int min_value = mutation->has_range_min ? mutation->range_min : default_min;
  const int max_value = mutation->has_range_max ? mutation->range_max : default_max;
  if (!value || min_value > max_value)
    return false;
  if (mutation->transform_name[0] && !nr_ue_fuzz_hook_text_eq(mutation->transform_name, "domain_value_selection")
      && !nr_ue_fuzz_hook_text_eq(mutation->transform_name, "runtime_echoed_mismatch"))
    return false;

  long chosen = *value;
  if (nr_ue_fuzz_hook_text_eq(mode, "boundary_min")) {
    chosen = min_value;
  } else if (nr_ue_fuzz_hook_text_eq(mode, "boundary_max")) {
    chosen = max_value;
  } else if (!mode || !*mode || nr_ue_fuzz_hook_text_eq(mode, "mismatch_in_range")) {
    const int span = max_value - min_value + 1;
    const int64_t offset = hook->txn_offset == 0 ? 1 : hook->txn_offset;
    chosen = min_value + (((int64_t)*value - min_value + offset) % span + span) % span;
    if (chosen == *value && span > 1)
      chosen = min_value + (chosen - min_value + 1) % span;
  } else if (nr_ue_fuzz_hook_text_eq(mode, "set_to_value")) {
    if (!mutation->override_value[0])
      return false;
    char *end = NULL;
    chosen = strtol(mutation->override_value, &end, 0);
    if (!end || *end)
      return false;
  } else {
    return false;
  }
  if (chosen == *value)
    return false;
  if (!nr_ue_fuzz_hook_register_integer_encode_patch(ue, value, chosen, min_value, max_value))
    return false;
  *value = chosen;
  return true;
}

static bool nr_ue_fuzz_hook_apply_txn_value(NR_UE_RRC_INST_t *ue, long *value, const char *mode)
{
  return nr_ue_fuzz_hook_apply_integer_value(ue, value, mode, 0, 3, true);
}

#define NR_GNB_TXN_ADAPTER(name, type)                                                                           \
  static bool name(NR_UE_RRC_INST_t *ue, void *payload, const char *mode)                                        \
  {                                                                                                              \
    return payload && nr_ue_fuzz_hook_apply_txn_value(ue, &((type *)payload)->rrc_TransactionIdentifier, mode);  \
  }

NR_GNB_TXN_ADAPTER(nr_gnb_txn_reconfiguration, NR_RRCReconfiguration_t)
NR_GNB_TXN_ADAPTER(nr_gnb_txn_security, NR_SecurityModeCommand_t)
NR_GNB_TXN_ADAPTER(nr_gnb_txn_capability, NR_UECapabilityEnquiry_t)
NR_GNB_TXN_ADAPTER(nr_gnb_txn_reestablishment, NR_RRCReestablishment_t)
NR_GNB_TXN_ADAPTER(nr_gnb_txn_release, NR_RRCRelease_t)
#undef NR_GNB_TXN_ADAPTER

static const nr_ue_fuzz_hook_field_adapter_t builtin_field_adapters[] = {
    {.target_msg = NR_UE_HOOK_MSG_RRC_RECONFIGURATION,
     .message_name = "RRCReconfiguration",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_gnb_txn_reconfiguration},
    {.target_msg = NR_UE_HOOK_MSG_SECURITY_MODE_COMMAND,
     .message_name = "SecurityModeCommand",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_gnb_txn_security},
    {.target_msg = NR_UE_HOOK_MSG_UE_CAPABILITY_ENQUIRY,
     .message_name = "UECapabilityEnquiry",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_gnb_txn_capability},
    {.target_msg = NR_UE_HOOK_MSG_RRC_REESTABLISHMENT,
     .message_name = "RRCReestablishment",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_gnb_txn_reestablishment},
    {.target_msg = NR_UE_HOOK_MSG_RRC_RELEASE,
     .message_name = "RRCRelease",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_gnb_txn_release},
};

#if defined(__has_include)
#if __has_include("nr_gnb_fuzz_hook_auto_generated.inc.c")
#define NR_GNB_FUZZ_HOOK_HAS_AUTO_GENERATED_ADAPTERS 1
#endif
#endif

#define ue_id rrc_ue_id
#ifdef NR_GNB_FUZZ_HOOK_HAS_AUTO_GENERATED_ADAPTERS
#include "nr_gnb_fuzz_hook_auto_generated.inc.c"
#define NR_GNB_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT \
  (sizeof(auto_generated_field_adapters) / sizeof(auto_generated_field_adapters[0]))
#else
static const nr_ue_fuzz_hook_field_adapter_t auto_generated_field_adapters[1] = {{0}};
#define NR_GNB_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT 0
#endif
#undef ue_id

static const nr_ue_fuzz_hook_field_adapter_t *nr_ue_fuzz_hook_find_field_adapter_for_mutation(
    const nr_ue_fuzz_hook_state_t *hook,
    const nr_ue_fuzz_hook_field_mutation_t *mutation)
{
  if (!hook || !mutation || !mutation->enabled)
    return NULL;
  const bool has_adapter_key = mutation->adapter_key[0] != '\0';
  const bool has_domain_id = mutation->domain_id[0] != '\0';
  const nr_ue_fuzz_hook_field_adapter_t *found = NULL;
  const struct {
    const nr_ue_fuzz_hook_field_adapter_t *entries;
    size_t count;
  } catalogs[] = {
      {builtin_field_adapters, sizeof(builtin_field_adapters) / sizeof(builtin_field_adapters[0])},
      {auto_generated_field_adapters, NR_GNB_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT},
  };

  for (size_t c = 0; c < sizeof(catalogs) / sizeof(catalogs[0]); ++c) {
    for (size_t i = 0; i < catalogs[c].count; ++i) {
      const nr_ue_fuzz_hook_field_adapter_t *adapter = &catalogs[c].entries[i];
      if (adapter->target_msg != hook->target_msg)
        continue;
      if (has_adapter_key) {
        if (nr_ue_fuzz_hook_text_eq(mutation->adapter_key, adapter->adapter_key))
          return adapter;
        continue;
      }
      if (has_domain_id) {
        if (nr_ue_fuzz_hook_text_eq(mutation->domain_id, adapter->domain_id)
            && nr_ue_fuzz_hook_text_eq(mutation->operator_family, adapter->operator_family))
          return adapter;
        continue;
      }
      if (!nr_ue_fuzz_hook_text_eq(mutation->message, adapter->message_name)
          || !nr_ue_fuzz_hook_text_eq(mutation->field, adapter->field_name)
          || !nr_ue_fuzz_hook_text_eq(mutation->operator_family, adapter->operator_family))
        continue;
      if (found)
        return NULL;
      found = adapter;
    }
  }
  return found;
}

static void *nr_gnb_fuzz_hook_message_payload(NR_DL_DCCH_Message_t *pdu, nr_ue_fuzz_hook_msg_t *msg)
{
  *msg = NR_UE_HOOK_MSG_NONE;
  if (!pdu || pdu->message.present != NR_DL_DCCH_MessageType_PR_c1 || !pdu->message.choice.c1)
    return NULL;
  struct NR_DL_DCCH_MessageType__c1 *c1 = pdu->message.choice.c1;
#define NR_GNB_PAYLOAD_CASE(id, member)        \
  case NR_DL_DCCH_MessageType__c1_PR_##member: \
    *msg = id;                                 \
    return c1->choice.member
  switch (c1->present) {
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_RECONFIGURATION, rrcReconfiguration);
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_SECURITY_MODE_COMMAND, securityModeCommand);
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_UE_CAPABILITY_ENQUIRY, ueCapabilityEnquiry);
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_DL_INFORMATION_TRANSFER, dlInformationTransfer);
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_REESTABLISHMENT, rrcReestablishment);
    NR_GNB_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_RELEASE, rrcRelease);
    default:
      return NULL;
  }
#undef NR_GNB_PAYLOAD_CASE
}

static int nr_gnb_fuzz_hook_payload_txn(nr_ue_fuzz_hook_msg_t msg, void *payload)
{
  if (!payload)
    return -1;
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION:
      return ((NR_RRCReconfiguration_t *)payload)->rrc_TransactionIdentifier;
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMMAND:
      return ((NR_SecurityModeCommand_t *)payload)->rrc_TransactionIdentifier;
    case NR_UE_HOOK_MSG_UE_CAPABILITY_ENQUIRY:
      return ((NR_UECapabilityEnquiry_t *)payload)->rrc_TransactionIdentifier;
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT:
      return ((NR_RRCReestablishment_t *)payload)->rrc_TransactionIdentifier;
    case NR_UE_HOOK_MSG_RRC_RELEASE:
      return ((NR_RRCRelease_t *)payload)->rrc_TransactionIdentifier;
    default:
      return -1;
  }
}

static void nr_gnb_fuzz_hook_prepare_txn(nr_ue_fuzz_hook_state_t *hook)
{
  nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutation;
  if (hook->action != NR_UE_HOOK_ACTION_MUTATE_TXN || hook->field_mutation_count > 0 || mutation->enabled || mutation->message[0]
      || mutation->field[0] || mutation->operator_family[0] || mutation->transform_name[0] || mutation->selected_mode[0]
      || mutation->has_range_min || mutation->has_range_max)
    return;
  mutation->enabled = true;
  nr_ue_fuzz_hook_copy_text(mutation->message, sizeof(mutation->message), nr_ue_fuzz_hook_msg_name(hook->target_msg));
  nr_ue_fuzz_hook_copy_text(mutation->field, sizeof(mutation->field), "transactionIdentifier");
  nr_ue_fuzz_hook_copy_text(mutation->operator_family, sizeof(mutation->operator_family), "integer_transform");
  nr_ue_fuzz_hook_copy_text(mutation->transform_name, sizeof(mutation->transform_name), "domain_value_selection");
}

static bool nr_gnb_fuzz_hook_apply_mutations(NR_UE_RRC_INST_t *ue, nr_ue_fuzz_hook_msg_t msg, void *payload)
{
  if (!ue || !payload)
    return false;
  nr_ue_fuzz_hook_clear_integer_encode_patches(ue);
  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  if (!hook->enabled || msg != hook->target_msg)
    return false;
  nr_gnb_fuzz_hook_prepare_txn(hook);
  if (hook->action != NR_UE_HOOK_ACTION_MUTATE_FIELD && hook->action != NR_UE_HOOK_ACTION_MUTATE_TXN)
    return false;

  const unsigned int original_field_mutation_count = hook->field_mutation_count;
  const nr_ue_fuzz_hook_field_mutation_t *mutations[NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS] = {0};
  const nr_ue_fuzz_hook_field_adapter_t *adapters[NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS] = {0};
  unsigned int mutation_count = 0;
  if (hook->field_mutation_count > 0) {
    for (unsigned int i = 0; i < hook->field_mutation_count && i < NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS; ++i) {
      if (hook->field_mutations[i].enabled)
        mutations[mutation_count++] = &hook->field_mutations[i];
    }
  } else if (hook->field_mutation.enabled) {
    mutations[mutation_count++] = &hook->field_mutation;
  }

  hook->field_mutation_applied_count = 0;
  hook->field_mutation_failed_index = 0;
  hook->field_mutation_result[0] = '\0';
  if (mutation_count == 0)
    return false;
  if (hook->action == NR_UE_HOOK_ACTION_MUTATE_TXN && mutation_count != 1)
    return false;

  for (unsigned int i = 0; i < mutation_count; ++i) {
    const nr_ue_fuzz_hook_field_mutation_t *mutation = mutations[i];
    if (hook->action == NR_UE_HOOK_ACTION_MUTATE_TXN
        && (!nr_ue_fuzz_hook_text_eq(mutation->field, "transactionIdentifier")
            || !nr_ue_fuzz_hook_text_eq(mutation->operator_family, "integer_transform")))
      return false;
    adapters[i] = nr_ue_fuzz_hook_find_field_adapter_for_mutation(hook, mutation);
    if (!adapters[i]) {
      hook->field_mutation_failed_index = i + 1;
      nr_ue_fuzz_hook_copy_text(hook->field_mutation_result, sizeof(hook->field_mutation_result), "adapter_not_found");
      nr_ue_rrc_trace_adapter(ue,
                              mutation->message,
                              mutation->field,
                              mutation->operator_family,
                              mutation->selected_mode,
                              "adapter_not_found");
      nr_ue_fuzz_hook_write_state(ue);
      return false;
    }
  }

  nr_ue_fuzz_hook_field_mutation_t applied_mutations[NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS] = {0};
  for (unsigned int i = 0; i < mutation_count; ++i) {
    const nr_ue_fuzz_hook_field_mutation_t *mutation = mutations[i];
    const nr_ue_fuzz_hook_field_adapter_t *adapter = adapters[i];
    hook->field_mutation = *mutation;
    if (!adapter->apply(ue, payload, mutation->selected_mode)) {
      hook->field_mutation_failed_index = i + 1;
      nr_ue_fuzz_hook_copy_text(hook->field_mutation_result, sizeof(hook->field_mutation_result), "apply_failed");
      nr_ue_rrc_trace_adapter(ue,
                              adapter->message_name,
                              adapter->field_name,
                              adapter->operator_family,
                              mutation->selected_mode,
                              "apply_failed");
      nr_ue_fuzz_hook_write_state(ue);
      return false;
    }
    hook->field_mutation_applied_count++;
    applied_mutations[i] = *mutation;
    nr_ue_rrc_trace_adapter(ue,
                            adapter->message_name,
                            adapter->field_name,
                            adapter->operator_family,
                            mutation->selected_mode,
                            "applied");
  }

  nr_ue_fuzz_hook_copy_text(hook->field_mutation_result, sizeof(hook->field_mutation_result), "applied");
  hook->field_mutation_count = original_field_mutation_count;
  if (original_field_mutation_count > 0) {
    for (unsigned int i = 0; i < original_field_mutation_count && i < NR_UE_FUZZ_HOOK_MAX_FIELD_MUTATIONS; ++i)
      hook->field_mutations[i] = applied_mutations[i];
  } else if (mutation_count == 1) {
    hook->field_mutation = applied_mutations[0];
  }
  return true;
}

static bool nr_gnb_fuzz_hook_process_dl_dcch(NR_UE_RRC_INST_t *ue,
                                             int srb_id,
                                             const uint8_t *buffer,
                                             int size,
                                             uint8_t *mutated_buffer,
                                             int mutated_buffer_size,
                                             int *mutated_size,
                                             uint8_t *repeat_buffer,
                                             int repeat_buffer_size,
                                             int *repeat_size)
{
  *mutated_size = 0;
  *repeat_size = 0;
  nr_ue_fuzz_hook_reload_config(ue);

  nr_ue_fuzz_hook_state_t *hook = &ue->fuzz_hook;
  uint8_t previous_buffer[NR_RRC_BUF_SIZE] = {0};
  const int previous_size = hook->last_dl_size > 0 && hook->last_dl_size <= NR_RRC_BUF_SIZE ? hook->last_dl_size : 0;
  if (previous_size > 0)
    memcpy(previous_buffer, hook->last_dl_pdu, previous_size);

  NR_DL_DCCH_Message_t *dl_dcch_msg = NULL;
  asn_dec_rval_t dec = uper_decode(NULL, &asn_DEF_NR_DL_DCCH_Message, (void **)&dl_dcch_msg, buffer, size, 0, 0);
  if (dec.code != RC_OK || !dl_dcch_msg) {
    nr_ue_fuzz_hook_copy_text(hook->field_mutation_result, sizeof(hook->field_mutation_result), "decode_failed");
    nr_ue_fuzz_hook_write_state(ue);
    return true;
  }

  nr_ue_fuzz_hook_msg_t msg = NR_UE_HOOK_MSG_NONE;
  void *payload = nr_gnb_fuzz_hook_message_payload(dl_dcch_msg, &msg);
  const int txn = nr_gnb_fuzz_hook_payload_txn(msg, payload);
  nr_gnb_fuzz_hook_record_dl(ue, msg, txn, buffer, size);

  bool should_send = true;
  const bool hit_target = hook->enabled && hook->target_msg == msg;
  if (!hit_target)
    goto done;

  if (hook->action == NR_UE_HOOK_ACTION_DROP) {
    LOG_W(NR_RRC, "[gNB][HOOK] UE %u drop %s on SRB%d\n", ue->rrc_ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id);
    nr_ue_fuzz_hook_record_fire(ue, msg, NR_UE_HOOK_ACTION_DROP, srb_id);
    should_send = false;
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(ue);
    nr_ue_fuzz_hook_write_state(ue);
    goto done;
  }

  if (hook->action == NR_UE_HOOK_ACTION_DELAY) {
    const int delay_ms = hook->delay_ms > 0 ? hook->delay_ms : 50;
    LOG_W(NR_RRC, "[gNB][HOOK] UE %u delay %s on SRB%d by %d ms\n", ue->rrc_ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id, delay_ms);
    usleep((useconds_t)delay_ms * 1000);
    nr_ue_fuzz_hook_record_fire(ue, msg, NR_UE_HOOK_ACTION_DELAY, srb_id);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(ue);
    nr_ue_fuzz_hook_write_state(ue);
    goto done;
  }

  if (hook->action == NR_UE_HOOK_ACTION_DUPLICATE) {
    const int copy_size = size < repeat_buffer_size ? size : repeat_buffer_size;
    if (copy_size > 0) {
      memcpy(repeat_buffer, buffer, copy_size);
      *repeat_size = copy_size;
    }
    LOG_W(NR_RRC, "[gNB][HOOK] UE %u duplicate %s on SRB%d\n", ue->rrc_ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id);
    nr_ue_fuzz_hook_record_fire(ue, msg, NR_UE_HOOK_ACTION_DUPLICATE, srb_id);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(ue);
    nr_ue_fuzz_hook_write_state(ue);
    goto done;
  }

  if (hook->action == NR_UE_HOOK_ACTION_REPLAY) {
    const int copy_size = previous_size < repeat_buffer_size ? previous_size : repeat_buffer_size;
    if (copy_size > 0) {
      memcpy(repeat_buffer, previous_buffer, copy_size);
      *repeat_size = copy_size;
      LOG_W(NR_RRC, "[gNB][HOOK] UE %u replay previous DL-DCCH after %s on SRB%d\n", ue->rrc_ue_id, nr_ue_fuzz_hook_msg_name(msg), srb_id);
      nr_ue_fuzz_hook_record_fire(ue, msg, NR_UE_HOOK_ACTION_REPLAY, srb_id);
      if (hook->arm_once)
        nr_ue_fuzz_hook_disarm_persistent(ue);
      nr_ue_fuzz_hook_write_state(ue);
    }
    goto done;
  }

  if (hook->action == NR_UE_HOOK_ACTION_MUTATE_FIELD || hook->action == NR_UE_HOOK_ACTION_MUTATE_TXN) {
    if (!nr_gnb_fuzz_hook_apply_mutations(ue, msg, payload))
      goto done;
    memset(mutated_buffer, 0, mutated_buffer_size);
    asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_DL_DCCH_Message, NULL, dl_dcch_msg, mutated_buffer, mutated_buffer_size);
    if (enc.encoded <= 0) {
      nr_ue_fuzz_hook_copy_text(hook->field_mutation_result, sizeof(hook->field_mutation_result), "encode_failed");
      nr_ue_fuzz_hook_write_state(ue);
      goto done;
    }
    *mutated_size = (enc.encoded + 7) / 8;
    nr_ue_fuzz_hook_record_fire(ue, msg, hook->action, srb_id);
    LOG_W(NR_RRC,
          "[gNB][HOOK] UE %u mutated %s on SRB%d bytes %d -> %d\n",
          ue->rrc_ue_id,
          nr_ue_fuzz_hook_msg_name(msg),
          srb_id,
          size,
          *mutated_size);
    if (hook->arm_once)
      nr_ue_fuzz_hook_disarm_persistent(ue);
    nr_ue_fuzz_hook_write_state(ue);
  }

done:
  ASN_STRUCT_FREE(asn_DEF_NR_DL_DCCH_Message, dl_dcch_msg);
  return should_send;
}

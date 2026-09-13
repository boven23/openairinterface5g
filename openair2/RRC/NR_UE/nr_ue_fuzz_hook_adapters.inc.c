/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* Included by rrc_UE.c; all field paths are owned by registered adapters. */

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

static bool nr_ue_fuzz_hook_text_eq(const char *lhs, const char *rhs)
{
  if (!lhs || !rhs)
    return false;
  return !strcasecmp(lhs, rhs);
}

typedef long **(*nr_ue_fuzz_hook_optional_enum_locator_fn_t)(NR_RRCReconfigurationComplete_t *reconfComplete, bool create);
typedef BOOLEAN_t **(*nr_ue_fuzz_hook_optional_boolean_locator_fn_t)(NR_RRCReconfigurationComplete_t *reconfComplete, bool create);

static NR_RRCReconfigurationComplete_v1610_IEs_t *nr_ue_fuzz_hook_get_reconfig_complete_v1610(
    NR_RRCReconfigurationComplete_t *reconfComplete)
{
  if (!reconfComplete
      || reconfComplete->criticalExtensions.present
             != NR_RRCReconfigurationComplete__criticalExtensions_PR_rrcReconfigurationComplete)
    return NULL;

  NR_RRCReconfigurationComplete_IEs_t *ies = reconfComplete->criticalExtensions.choice.rrcReconfigurationComplete;
  if (!ies || !ies->nonCriticalExtension || !ies->nonCriticalExtension->nonCriticalExtension
      || !ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension) {
    return NULL;
  }

  return ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension;
}

static NR_RRCReconfigurationComplete_v1610_IEs_t *nr_ue_fuzz_hook_ensure_reconfig_complete_v1610(
    NR_RRCReconfigurationComplete_t *reconfComplete)
{
  if (!reconfComplete
      || reconfComplete->criticalExtensions.present
             != NR_RRCReconfigurationComplete__criticalExtensions_PR_rrcReconfigurationComplete
      || !reconfComplete->criticalExtensions.choice.rrcReconfigurationComplete)
    return NULL;
  NR_RRCReconfigurationComplete_IEs_t *ies = reconfComplete->criticalExtensions.choice.rrcReconfigurationComplete;
  if (!ies->nonCriticalExtension)
    ies->nonCriticalExtension = CALLOC(1, sizeof(*ies->nonCriticalExtension));
  if (!ies->nonCriticalExtension->nonCriticalExtension)
    ies->nonCriticalExtension->nonCriticalExtension = CALLOC(1, sizeof(*ies->nonCriticalExtension->nonCriticalExtension));
  if (!ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension)
    ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension =
        CALLOC(1, sizeof(*ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension));
  return ies->nonCriticalExtension->nonCriticalExtension->nonCriticalExtension;
}

static NR_UE_MeasurementsAvailable_r16_t *nr_ue_fuzz_hook_ensure_ue_measurements_available(
    NR_RRCReconfigurationComplete_t *reconfComplete)
{
  NR_RRCReconfigurationComplete_v1610_IEs_t *v1610 = nr_ue_fuzz_hook_ensure_reconfig_complete_v1610(reconfComplete);
  if (!v1610)
    return NULL;
  if (!v1610->ue_MeasurementsAvailable_r16)
    v1610->ue_MeasurementsAvailable_r16 = CALLOC(1, sizeof(*v1610->ue_MeasurementsAvailable_r16));
  return v1610->ue_MeasurementsAvailable_r16;
}

static NR_UE_MeasurementsAvailable_r16_t *nr_ue_fuzz_hook_get_ue_measurements_available(
    NR_RRCReconfigurationComplete_t *reconfComplete)
{
  NR_RRCReconfigurationComplete_v1610_IEs_t *v1610 = nr_ue_fuzz_hook_get_reconfig_complete_v1610(reconfComplete);
  return v1610 ? v1610->ue_MeasurementsAvailable_r16 : NULL;
}

static long **nr_ue_fuzz_hook_locate_logmeasavailable_slot(NR_RRCReconfigurationComplete_t *reconfComplete, bool create)
{
  NR_UE_MeasurementsAvailable_r16_t *meas = create ? nr_ue_fuzz_hook_ensure_ue_measurements_available(reconfComplete)
                                                   : nr_ue_fuzz_hook_get_ue_measurements_available(reconfComplete);
  if (!meas)
    return NULL;
  return &meas->logMeasAvailable_r16;
}

static BOOLEAN_t **nr_ue_fuzz_hook_locate_siglogmeasconfigavailable_slot(NR_RRCReconfigurationComplete_t *reconfComplete,
                                                                         bool create)
{
  NR_UE_MeasurementsAvailable_r16_t *meas = create ? nr_ue_fuzz_hook_ensure_ue_measurements_available(reconfComplete)
                                                   : nr_ue_fuzz_hook_get_ue_measurements_available(reconfComplete);
  if (!meas)
    return NULL;

  if (create && !meas->ext1)
    meas->ext1 = CALLOC(1, sizeof(*meas->ext1));
  if (!meas->ext1)
    return NULL;

  return &meas->ext1->sigLogMeasConfigAvailable_r17;
}

static bool nr_ue_fuzz_hook_apply_optional_singleton_enum(NR_UE_RRC_INST_t *rrc,
                                                          NR_RRCReconfigurationComplete_t *reconfComplete,
                                                          const char *mode,
                                                          const char *default_mode,
                                                          const char *field_name,
                                                          nr_ue_fuzz_hook_optional_enum_locator_fn_t locate_slot,
                                                          long true_value)
{
  if (!locate_slot)
    return false;

  if (!mode || !*mode)
    mode = default_mode;

  if (!strcasecmp(mode, "force_present_true")) {
    long **slot = locate_slot(reconfComplete, true);
    if (!slot)
      return false;
    if (!*slot)
      *slot = CALLOC(1, sizeof(**slot));
    **slot = true_value;
    LOG_W(NR_RRC, "[UE %ld][HOOK] force %s in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  if (!strcasecmp(mode, "omit")) {
    long **slot = locate_slot(reconfComplete, false);
    if (!slot || !*slot)
      return false;
    free(*slot);
    *slot = NULL;
    LOG_W(NR_RRC, "[UE %ld][HOOK] omit %s in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  return false;
}

static bool nr_ue_fuzz_hook_apply_optional_boolean_assignment(NR_UE_RRC_INST_t *rrc,
                                                              NR_RRCReconfigurationComplete_t *reconfComplete,
                                                              const char *mode,
                                                              const char *default_mode,
                                                              const char *field_name,
                                                              nr_ue_fuzz_hook_optional_boolean_locator_fn_t locate_slot)
{
  if (!locate_slot)
    return false;

  if (!mode || !*mode)
    mode = default_mode;

  if (!strcasecmp(mode, "force_true")) {
    BOOLEAN_t **slot = locate_slot(reconfComplete, true);
    if (!slot)
      return false;
    if (!*slot)
      *slot = CALLOC(1, sizeof(**slot));
    **slot = 1;
    LOG_W(NR_RRC, "[UE %ld][HOOK] force %s=true in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  if (!strcasecmp(mode, "force_false")) {
    BOOLEAN_t **slot = locate_slot(reconfComplete, true);
    if (!slot)
      return false;
    if (!*slot)
      *slot = CALLOC(1, sizeof(**slot));
    **slot = 0;
    LOG_W(NR_RRC, "[UE %ld][HOOK] force %s=false in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  if (!strcasecmp(mode, "set_to_value")) {
    const char *override_value = rrc->fuzz_hook.field_mutation.override_value;
    if (!override_value || !*override_value)
      return false;
    BOOLEAN_t **slot = locate_slot(reconfComplete, true);
    if (!slot)
      return false;
    if (!*slot)
      *slot = CALLOC(1, sizeof(**slot));
    if (!strcasecmp(override_value, "true") || !strcmp(override_value, "1"))
      **slot = 1;
    else if (!strcasecmp(override_value, "false") || !strcmp(override_value, "0"))
      **slot = 0;
    else
      return false;
    LOG_W(NR_RRC, "[UE %ld][HOOK] set %s from override in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  if (!strcasecmp(mode, "omit")) {
    BOOLEAN_t **slot = locate_slot(reconfComplete, false);
    if (!slot || !*slot)
      return false;
    free(*slot);
    *slot = NULL;
    LOG_W(NR_RRC, "[UE %ld][HOOK] omit %s in RRCReconfigurationComplete\n", rrc->ue_id, field_name);
    return true;
  }

  return false;
}

static bool nr_ue_fuzz_hook_apply_logmeas_presence_adapter(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  return nr_ue_fuzz_hook_apply_optional_singleton_enum(rrc,
                                                       (NR_RRCReconfigurationComplete_t *)payload,
                                                       mode,
                                                       "force_present_true",
                                                       "logMeasAvailable",
                                                       nr_ue_fuzz_hook_locate_logmeasavailable_slot,
                                                       NR_UE_MeasurementsAvailable_r16__logMeasAvailable_r16_true);
}

static bool nr_ue_fuzz_hook_apply_siglog_boolean_adapter(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  return nr_ue_fuzz_hook_apply_optional_boolean_assignment(rrc,
                                                           (NR_RRCReconfigurationComplete_t *)payload,
                                                           mode,
                                                           "force_true",
                                                           "sigLogMeasConfigAvailable",
                                                           nr_ue_fuzz_hook_locate_siglogmeasconfigavailable_slot);
}

static bool nr_ue_fuzz_hook_apply_txn_value(NR_UE_RRC_INST_t *rrc, long *value, const char *mode)
{
  const nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  const nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutation;
  const int min_value = mutation->has_range_min ? mutation->range_min : 0;
  const int max_value = mutation->has_range_max ? mutation->range_max : 3;
  if (!value || min_value < 0 || max_value > 3 || min_value > max_value)
    return false;

  const bool echoed = nr_ue_fuzz_hook_text_eq(mutation->transform_name, "runtime_echoed_mismatch");
  if (mutation->transform_name[0] && !echoed && !nr_ue_fuzz_hook_text_eq(mutation->transform_name, "domain_value_selection"))
    return false;
  const long reference = echoed && hook->last_dl_txn >= 0 ? hook->last_dl_txn : *value;
  long chosen;
  if (nr_ue_fuzz_hook_text_eq(mode, "boundary_min")) {
    chosen = min_value;
    if (echoed && chosen == reference)
      chosen = max_value;
  } else if (nr_ue_fuzz_hook_text_eq(mode, "boundary_max")) {
    chosen = max_value;
    if (echoed && chosen == reference)
      chosen = min_value;
  } else if (!mode || !*mode || nr_ue_fuzz_hook_text_eq(mode, "mismatch_in_range")) {
    const int span = max_value - min_value + 1;
    const int64_t offset = mutation->transform_name[0] && hook->txn_offset == 0 ? 1 : hook->txn_offset;
    chosen = min_value + (((int64_t)reference - min_value + offset) % span + span) % span;
    if (echoed && chosen == reference && span > 1)
      chosen = min_value + (chosen - min_value + 1) % span;
  } else if (nr_ue_fuzz_hook_text_eq(mode, "set_to_value")) {
    if (!mutation->override_value[0])
      return false;
    char *end = NULL;
    chosen = strtol(mutation->override_value, &end, 0);
    if (!end || *end)
      return false;
    if (chosen < min_value || chosen > max_value)
      return false;
  } else {
    return false;
  }
  if (chosen == *value)
    return false;
  *value = chosen;
  return true;
}

#define NR_UE_TXN_ADAPTER(name, type)                                                                            \
  static bool name(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)                                       \
  {                                                                                                              \
    return payload && nr_ue_fuzz_hook_apply_txn_value(rrc, &((type *)payload)->rrc_TransactionIdentifier, mode); \
  }

NR_UE_TXN_ADAPTER(nr_ue_txn_setup, NR_RRCSetupComplete_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_security, NR_SecurityModeComplete_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_reconfiguration, NR_RRCReconfigurationComplete_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_reestablishment, NR_RRCReestablishmentComplete_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_capability, NR_UECapabilityInformation_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_dl_reconfiguration, NR_RRCReconfiguration_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_dl_security, NR_SecurityModeCommand_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_dl_capability, NR_UECapabilityEnquiry_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_dl_reestablishment, NR_RRCReestablishment_t)
NR_UE_TXN_ADAPTER(nr_ue_txn_dl_release, NR_RRCRelease_t)
#undef NR_UE_TXN_ADAPTER

static bool nr_ue_fuzz_hook_apply_nas_payload(NR_UE_RRC_INST_t *rrc, void *payload, const char *mode)
{
  (void)rrc;
  NR_ULInformationTransfer_t *transfer = payload;
  if (!transfer || transfer->criticalExtensions.present != NR_ULInformationTransfer__criticalExtensions_PR_ulInformationTransfer
      || !transfer->criticalExtensions.choice.ulInformationTransfer)
    return false;
  OCTET_STRING_t **slot = &transfer->criticalExtensions.choice.ulInformationTransfer->dedicatedNAS_Message;
  if (nr_ue_fuzz_hook_text_eq(mode, "omit")) {
    if (!*slot)
      return false;
    ASN_STRUCT_FREE(asn_DEF_OCTET_STRING, *slot);
    *slot = NULL;
    return true;
  }
  if (!nr_ue_fuzz_hook_text_eq(mode, "force_zero") && !nr_ue_fuzz_hook_text_eq(mode, "force_ff"))
    return false;
  const char value = nr_ue_fuzz_hook_text_eq(mode, "force_ff") ? (char)0xff : 0;
  if (*slot && (*slot)->size == 1 && (*slot)->buf && (*slot)->buf[0] == (uint8_t)value)
    return false;
  if (!*slot)
    *slot = CALLOC(1, sizeof(**slot));
  return *slot && OCTET_STRING_fromBuf(*slot, &value, 1) == 0;
}

static const nr_ue_fuzz_hook_field_adapter_t builtin_field_adapters[] = {
    {.target_msg = NR_UE_HOOK_MSG_DL_RRC_RECONFIGURATION,
     .message_name = "DL_RRCReconfiguration",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_dl_reconfiguration},
    {.target_msg = NR_UE_HOOK_MSG_DL_SECURITY_MODE_COMMAND,
     .message_name = "DL_SecurityModeCommand",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_dl_security},
    {.target_msg = NR_UE_HOOK_MSG_DL_UE_CAPABILITY_ENQUIRY,
     .message_name = "DL_UECapabilityEnquiry",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_dl_capability},
    {.target_msg = NR_UE_HOOK_MSG_DL_RRC_REESTABLISHMENT,
     .message_name = "DL_RRCReestablishment",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_dl_reestablishment},
    {.target_msg = NR_UE_HOOK_MSG_DL_RRC_RELEASE,
     .message_name = "DL_RRCRelease",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_dl_release},
    {.target_msg = NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER,
     .message_name = "ULInformationTransfer",
     .field_name = "dedicatedNAS-Message",
     .operator_family = "optional_octet_string_assignment",
     .apply = nr_ue_fuzz_hook_apply_nas_payload},
    {.target_msg = NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE,
     .message_name = "RRCSetupComplete",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_setup},
    {.target_msg = NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE,
     .message_name = "SecurityModeComplete",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_security},
    {.target_msg = NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE,
     .message_name = "RRCReconfigurationComplete",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_reconfiguration},
    {.target_msg = NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE,
     .message_name = "RRCReestablishmentComplete",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_reestablishment},
    {.target_msg = NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION,
     .message_name = "UECapabilityInformation",
     .field_name = "transactionIdentifier",
     .operator_family = "integer_transform",
     .apply = nr_ue_txn_capability},
    {
        .target_msg = NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE,
        .message_name = "RRCReconfigurationComplete",
        .field_name = "logMeasAvailable",
        .operator_family = "optional_presence_toggle",
        .apply = nr_ue_fuzz_hook_apply_logmeas_presence_adapter,
    },
    {
        .target_msg = NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE,
        .message_name = "RRCReconfigurationComplete",
        .field_name = "sigLogMeasConfigAvailable",
        .operator_family = "optional_boolean_assignment",
        .apply = nr_ue_fuzz_hook_apply_siglog_boolean_adapter,
    },
};

#if defined(__has_include)
#  if __has_include("nr_ue_fuzz_hook_auto_generated.inc.c")
#    define NR_UE_FUZZ_HOOK_HAS_AUTO_GENERATED_ADAPTERS 1
#  endif
#endif

#ifdef NR_UE_FUZZ_HOOK_HAS_AUTO_GENERATED_ADAPTERS
#  include "nr_ue_fuzz_hook_auto_generated.inc.c"
#  define NR_UE_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT \
    (sizeof(auto_generated_field_adapters) / sizeof(auto_generated_field_adapters[0]))
#else
static const nr_ue_fuzz_hook_field_adapter_t auto_generated_field_adapters[1] = {{0}};
#  define NR_UE_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT 0
#endif

/* A distinct transform namespace keeps context adapters separate from generated
 * domain-selection adapters with the same field identity. */
static const nr_ue_fuzz_hook_field_adapter_t context_field_adapters[] = {
    {.target_msg = NR_UE_HOOK_MSG_MEASUREMENT_REPORT,
     .message_name = "MeasurementReport",
     .field_name = "measId",
     .operator_family = "integer_transform",
     .apply = nr_ue_hook_mutate_meas_context},
    {.target_msg = NR_UE_HOOK_MSG_MEASUREMENT_REPORT,
     .message_name = "MeasurementReport",
     .field_name = "measResults",
     .operator_family = "field_assignment",
     .apply = nr_ue_hook_mutate_meas_context},
    {.target_msg = NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE,
     .message_name = "RRCSetupComplete",
     .field_name = "selectedPLMN-Identity",
     .operator_family = "integer_transform",
     .apply = nr_ue_hook_mutate_plmn},
    {.target_msg = NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION,
     .message_name = "UECapabilityInformation",
     .field_name = "ue-CapabilityRAT-ContainerList",
     .operator_family = "field_assignment",
     .apply = nr_ue_hook_mutate_rat},
};

/* Resolve one adapter identity across both catalogs; do not silently shadow a path. */
static const nr_ue_fuzz_hook_field_adapter_t *nr_ue_fuzz_hook_find_field_adapter(const nr_ue_fuzz_hook_state_t *hook)
{
  if (!hook || !hook->field_mutation.enabled)
    return NULL;
  const nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutation;
  const bool has_adapter_key = mutation->adapter_key[0] != '\0';
  const bool has_domain_id = mutation->domain_id[0] != '\0';

  if (nr_ue_fuzz_hook_text_eq(mutation->transform_name, "runtime_context")) {
    for (size_t i = 0; i < sizeof(context_field_adapters) / sizeof(*context_field_adapters); i++) {
      const nr_ue_fuzz_hook_field_adapter_t *adapter = &context_field_adapters[i];
      if (adapter->target_msg == hook->target_msg && nr_ue_fuzz_hook_text_eq(mutation->message, adapter->message_name)
          && nr_ue_fuzz_hook_text_eq(mutation->field, adapter->field_name)
          && nr_ue_fuzz_hook_text_eq(mutation->operator_family, adapter->operator_family))
        return adapter;
    }
    return NULL;
  }

  const nr_ue_fuzz_hook_field_adapter_t *found = NULL;
  const struct {
    const nr_ue_fuzz_hook_field_adapter_t *entries;
    size_t count;
  } catalogs[] = {
      {builtin_field_adapters, sizeof(builtin_field_adapters) / sizeof(builtin_field_adapters[0])},
      {auto_generated_field_adapters, NR_UE_FUZZ_HOOK_AUTO_GENERATED_ADAPTER_COUNT},
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
      if (found) {
        LOG_W(NR_RRC,
              "[UE HOOK] duplicate adapter identity for %s.%s:%s\n",
              mutation->message,
              mutation->field,
              mutation->operator_family);
        return NULL;
      }
      found = adapter;
    }
  }
  return found;
}

/* This switch unwraps the message, never traverses a field path. */
static void *nr_ue_fuzz_hook_message_payload(NR_UL_DCCH_Message_t *pdu, nr_ue_fuzz_hook_msg_t *msg)
{
  *msg = NR_UE_HOOK_MSG_NONE;
  if (!pdu || pdu->message.present != NR_UL_DCCH_MessageType_PR_c1 || !pdu->message.choice.c1)
    return NULL;
  struct NR_UL_DCCH_MessageType__c1 *c1 = pdu->message.choice.c1;
#define NR_UE_PAYLOAD_CASE(id, member)         \
  case NR_UL_DCCH_MessageType__c1_PR_##member: \
    *msg = id;                                 \
    return c1->choice.member
  switch (c1->present) {
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE, rrcSetupComplete);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE, securityModeComplete);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE, rrcReconfigurationComplete);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE, rrcReestablishmentComplete);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION, ueCapabilityInformation);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER, ulInformationTransfer);
    NR_UE_PAYLOAD_CASE(NR_UE_HOOK_MSG_MEASUREMENT_REPORT, measurementReport);
    default:
      return NULL;
  }
#undef NR_UE_PAYLOAD_CASE
}

/* Legacy arm/oneshot controls supply only target, mutate_txn and txn_offset.
 * Supply defaults only when there is no explicit field contract. Never turn a
 * malformed/disabled field contract into a different, successful mutation. */
static void nr_ue_fuzz_hook_prepare_txn(nr_ue_fuzz_hook_state_t *hook)
{
  nr_ue_fuzz_hook_field_mutation_t *mutation = &hook->field_mutation;
  if (hook->action != NR_UE_HOOK_ACTION_MUTATE_TXN || mutation->enabled || mutation->message[0] || mutation->field[0]
      || mutation->operator_family[0] || mutation->transform_name[0] || mutation->selected_mode[0] || mutation->has_range_min
      || mutation->has_range_max)
    return;
  mutation->enabled = true;
  nr_ue_fuzz_hook_copy_text(mutation->message, sizeof(mutation->message), nr_ue_fuzz_hook_msg_name(hook->target_msg));
  nr_ue_fuzz_hook_copy_text(mutation->field, sizeof(mutation->field), "transactionIdentifier");
  nr_ue_fuzz_hook_copy_text(mutation->operator_family, sizeof(mutation->operator_family), "integer_transform");
  // Offset the current message's transaction, not a possibly unrelated last DL.
  nr_ue_fuzz_hook_copy_text(mutation->transform_name, sizeof(mutation->transform_name), "domain_value_selection");
}

/* Shared by UL before encoding and DL after decoding. DL integrity verification
 * continues to use the original received bytes; only the local RRC object changes. */
static bool nr_ue_fuzz_hook_mutate_payload(NR_UE_RRC_INST_t *rrc, nr_ue_fuzz_hook_msg_t msg, void *payload)
{
  if (!rrc || !payload)
    return false;
  nr_ue_fuzz_hook_reload_config(rrc);
  nr_ue_fuzz_hook_state_t *hook = &rrc->fuzz_hook;
  if (!hook->enabled || msg != hook->target_msg)
    return false;
  if (!nr_ue_hook_gates_ready(rrc)) {
    nr_ue_fuzz_hook_write_state(rrc);
    return false;
  }
  nr_ue_fuzz_hook_prepare_txn(hook);
  if (!hook->enabled || !hook->field_mutation.enabled
      || (hook->action != NR_UE_HOOK_ACTION_MUTATE_FIELD && hook->action != NR_UE_HOOK_ACTION_MUTATE_TXN))
    return false;
  if (hook->action == NR_UE_HOOK_ACTION_MUTATE_TXN
      && (!nr_ue_fuzz_hook_text_eq(hook->field_mutation.field, "transactionIdentifier")
          || !nr_ue_fuzz_hook_text_eq(hook->field_mutation.operator_family, "integer_transform")))
    return false;
  const nr_ue_fuzz_hook_field_adapter_t *adapter = nr_ue_fuzz_hook_find_field_adapter(hook);
  if (!adapter) {
    LOG_W(NR_RRC,
          "[UE %ld][HOOK] no unique adapter for key=%s %s.%s:%s\n",
          rrc->ue_id,
          hook->field_mutation.adapter_key,
          hook->field_mutation.message,
          hook->field_mutation.field,
          hook->field_mutation.operator_family);
    return false;
  }
  if (!adapter->apply(rrc, payload, hook->field_mutation.selected_mode)) {
    nr_ue_fuzz_hook_write_state(rrc);
    return false;
  }
  nr_ue_fuzz_hook_record_fire(rrc, msg, hook->action, -1);
  if (hook->arm_once)
    nr_ue_fuzz_hook_disarm_persistent(rrc);
  nr_ue_fuzz_hook_write_state(rrc);
  return true;
}

static bool nr_ue_fuzz_hook_mutate_ul_dcch(void *context, NR_UL_DCCH_Message_t *pdu)
{
  nr_ue_fuzz_hook_msg_t msg = NR_UE_HOOK_MSG_NONE;
  void *payload = nr_ue_fuzz_hook_message_payload(pdu, &msg);
  bool changed = nr_ue_fuzz_hook_mutate_payload(context, msg, payload);
  if (context && msg == NR_UE_HOOK_MSG_MEASUREMENT_REPORT && !changed)
    nr_ue_hook_cache_report(context, payload);
  return changed;
}

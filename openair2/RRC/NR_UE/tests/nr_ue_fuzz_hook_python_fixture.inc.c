/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* Python -> control file -> production C adapters -> UPER -> counted PDCP sink.
 * Contexts below are test fixtures, not evidence of a live gNB procedure. */
static int python_control_fixture(const char *root, const char *message, const char *scenario)
{
  NR_UE_RRC_INST_t rrc = {0};
  snprintf(rrc.fuzz_hook.control_path, sizeof(rrc.fuzz_hook.control_path), "%s/oai_nr_ue_hook_0.ctl", root);
  snprintf(rrc.fuzz_hook.state_path, sizeof(rrc.fuzz_hook.state_path), "%s/oai_nr_ue_hook_0.state", root);
  rrc.fuzz_hook.control_mtime = -1;
  rrc.fuzz_hook.control_mtime_nsec = -1;
  nr_ue_fuzz_hook_msg_t msg = nr_ue_fuzz_hook_msg_from_name(message);
  CHECK(msg >= NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE && msg <= NR_UE_HOOK_MSG_MEASUREMENT_REPORT);
  rrc.as_security_activated = strcmp(scenario, "closed_gate") != 0;
  rrc.fuzz_hook.submitted[NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE] = rrc.as_security_activated;
  rrc.perNB[0].SInfo.sib1_validity = 1;
  rrc.perNB[0].hook_plmn_count = 1;
  if (!strcmp(scenario, "pdcp_failure"))
    fail_pdcp_call = 1;

  NR_UL_DCCH_Message_t pdu = {0};
  pdu.message.present = NR_UL_DCCH_MessageType_PR_c1;
  pdu.message.choice.c1 = calloc(1, sizeof(*pdu.message.choice.c1));
  struct NR_UL_DCCH_MessageType__c1 *c1 = pdu.message.choice.c1;
#define MAKE_PAYLOAD(member, type)                                                        \
  c1->present = NR_UL_DCCH_MessageType__c1_PR_##member;                                   \
  c1->choice.member = calloc(1, sizeof(type##_t));                                        \
  c1->choice.member->criticalExtensions.present = type##__criticalExtensions_PR_##member; \
  c1->choice.member->criticalExtensions.choice.member = calloc(1, sizeof(*c1->choice.member->criticalExtensions.choice.member))
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE: {
      MAKE_PAYLOAD(rrcSetupComplete, NR_RRCSetupComplete);
      NR_RRCSetupComplete_IEs_t *ies = c1->choice.rrcSetupComplete->criticalExtensions.choice.rrcSetupComplete;
      ies->selectedPLMN_Identity = 1;
      CHECK(OCTET_STRING_fromBuf(&ies->dedicatedNAS_Message, "nas", 3) == 0);
      break;
    }
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE: {
      MAKE_PAYLOAD(securityModeComplete, NR_SecurityModeComplete);
      break;
    }
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE: {
      MAKE_PAYLOAD(rrcReconfigurationComplete, NR_RRCReconfigurationComplete);
      break;
    }
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE: {
      MAKE_PAYLOAD(rrcReestablishmentComplete, NR_RRCReestablishmentComplete);
      break;
    }
    case NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION: {
      MAKE_PAYLOAD(ueCapabilityInformation, NR_UECapabilityInformation);
      NR_UECapabilityEnquiry_t enquiry = {0};
      enquiry.criticalExtensions.present = NR_UECapabilityEnquiry__criticalExtensions_PR_ueCapabilityEnquiry;
      NR_UECapabilityEnquiry_IEs_t enquiry_ies = {0};
      enquiry.criticalExtensions.choice.ueCapabilityEnquiry = &enquiry_ies;
      NR_UE_CapabilityRAT_Request_t request = {.rat_Type = NR_RAT_Type_nr};
      CHECK(ASN_SEQUENCE_ADD(&enquiry_ies.ue_CapabilityRAT_RequestList.list, &request) == 0);
      nr_ue_hook_record_enquiry(&rrc, &enquiry);
      free(enquiry_ies.ue_CapabilityRAT_RequestList.list.array);
      NR_UECapabilityInformation_IEs_t *ies = c1->choice.ueCapabilityInformation->criticalExtensions.choice.ueCapabilityInformation;
      ies->ue_CapabilityRAT_ContainerList = calloc(1, sizeof(*ies->ue_CapabilityRAT_ContainerList));
      NR_UE_CapabilityRAT_Container_t *container = calloc(1, sizeof(*container));
      container->rat_Type = NR_RAT_Type_nr;
      CHECK(OCTET_STRING_fromBuf(&container->ue_CapabilityRAT_Container, "nr-test-blob", 12) == 0);
      CHECK(ASN_SEQUENCE_ADD(&ies->ue_CapabilityRAT_ContainerList->list, container) == 0);
      break;
    }
    case NR_UE_HOOK_MSG_UL_INFORMATION_TRANSFER: {
      MAKE_PAYLOAD(ulInformationTransfer, NR_ULInformationTransfer);
      NR_ULInformationTransfer_IEs_t *ies = c1->choice.ulInformationTransfer->criticalExtensions.choice.ulInformationTransfer;
      ies->dedicatedNAS_Message = calloc(1, sizeof(*ies->dedicatedNAS_Message));
      CHECK(OCTET_STRING_fromBuf(ies->dedicatedNAS_Message, "nas", 3) == 0);
      break;
    }
    case NR_UE_HOOK_MSG_MEASUREMENT_REPORT: {
      unsigned quantities = !strcmp(scenario, "unsupported_quantities") ? 7 : 1;
      configure_binding(&rrc.perNB[0], 1, 1, 1, NR_NR_RS_Type_ssb, quantities);
      configure_binding(&rrc.perNB[0], 2, 2, 2, NR_NR_RS_Type_ssb, 1);
      nr_ue_hook_snapshot_meas(&rrc, 0);
      if (!strcmp(scenario, "history")) {
        NR_MeasurementReport_t *old = make_report(2, NR_NR_RS_Type_ssb, 1);
        nr_ue_hook_cache_report(&rrc, old);
        ASN_STRUCT_FREE(asn_DEF_NR_MeasurementReport, old);
        ASN_STRUCT_FREE(asn_DEF_NR_MeasIdToAddMod, rrc.perNB[0].MeasId[2]);
        rrc.perNB[0].MeasId[2] = NULL;
        nr_ue_hook_snapshot_meas(&rrc, 0);
      }
      nr_ue_hook_record_submission(&rrc, NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE);
      c1->present = NR_UL_DCCH_MessageType__c1_PR_measurementReport;
      c1->choice.measurementReport = make_report(1, NR_NR_RS_Type_ssb, quantities);
      break;
    }
    default:
      abort();
  }
#undef MAKE_PAYLOAD
  nr_ue_fuzz_hook_reload_config(&rrc);
  char field[128];
  snprintf(field, sizeof(field), "%s", rrc.fuzz_hook.field_mutation.field);
  bool changed = nr_ue_fuzz_hook_mutate_ul_dcch(&rrc, &pdu);
  uint8_t bytes[NR_RRC_BUF_SIZE];
  asn_enc_rval_t enc = uper_encode_to_buffer(&asn_DEF_NR_UL_DCCH_Message, NULL, &pdu, bytes, sizeof(bytes));
  CHECK(enc.encoded > 0);
  NR_UL_DCCH_Message_t *decoded = NULL;
  CHECK(uper_decode(NULL, &asn_DEF_NR_UL_DCCH_Message, (void **)&decoded, bytes, (enc.encoded + 7) / 8, 0, 0).code == RC_OK);
  nr_ue_fuzz_hook_msg_t decoded_msg;
  void *payload = nr_ue_fuzz_hook_message_payload(decoded, &decoded_msg);
  CHECK(decoded_msg == msg && payload);
  long value = -1;
  switch (msg) {
    case NR_UE_HOOK_MSG_RRC_SETUP_COMPLETE:
      value = ((NR_RRCSetupComplete_t *)payload)->rrc_TransactionIdentifier;
      if (!strcmp(field, "selectedPLMN-Identity"))
        value = ((NR_RRCSetupComplete_t *)payload)->criticalExtensions.choice.rrcSetupComplete->selectedPLMN_Identity;
      break;
    case NR_UE_HOOK_MSG_SECURITY_MODE_COMPLETE:
      value = ((NR_SecurityModeComplete_t *)payload)->rrc_TransactionIdentifier;
      break;
    case NR_UE_HOOK_MSG_RRC_RECONFIGURATION_COMPLETE:
      value = ((NR_RRCReconfigurationComplete_t *)payload)->rrc_TransactionIdentifier;
      break;
    case NR_UE_HOOK_MSG_RRC_REESTABLISHMENT_COMPLETE:
      value = ((NR_RRCReestablishmentComplete_t *)payload)->rrc_TransactionIdentifier;
      break;
    case NR_UE_HOOK_MSG_UE_CAPABILITY_INFORMATION:
      value = ((NR_UECapabilityInformation_t *)payload)->rrc_TransactionIdentifier;
      if (!strcmp(field, "ue-CapabilityRAT-ContainerList"))
        value = ((NR_UECapabilityInformation_t *)payload)
                    ->criticalExtensions.choice.ueCapabilityInformation->ue_CapabilityRAT_ContainerList->list.array[0]
                    ->rat_Type;
      break;
    case NR_UE_HOOK_MSG_MEASUREMENT_REPORT:
      value = nr_ue_hook_meas_results(payload)->measId;
      break;
    default:
      break;
  }
  nr_ue_fuzz_hook_send_srb(&rrc, msg, 1, bytes, (enc.encoded + 7) / 8);
  printf("RESULT {\"changed\":%s,\"decoded_value\":%ld,\"encoded_bytes\":%ld,\"pdcp_calls\":%d}\n",
         changed ? "true" : "false",
         value,
         (enc.encoded + 7) / 8,
         sent_pdus);
  ASN_STRUCT_FREE(asn_DEF_NR_UL_DCCH_Message, decoded);
  ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_NR_UL_DCCH_Message, &pdu);
  nr_ue_hook_clear_context(&rrc);
  return 0;
}

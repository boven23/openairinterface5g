/* SPDX-License-Identifier: LicenseRef-CSSL-1.0 */
/* Shared measurement removal paths, also exercised by UE hook regression tests. */
#include "common/utils/oai_asn1.h"
#include <math.h>

static void handle_meas_reporting_remove(rrcPerNB_t *rrc, int id, NR_UE_Timers_Constants_t *timers)
{
  // remove the measurement reporting entry for this measId if included
  asn1cFreeStruc(asn_DEF_NR_VarMeasReport, rrc->MeasReport[id]);
  // TODO stop the periodical reporting timer or timer T321, whichever is running,
  // and reset the associated information (e.g. timeToTrigger) for this measId
  nr_timer_stop(&timers->T321);

  l3_measurements_t *l3_measurements = &rrc->l3_measurements;
  nr_timer_stop(&l3_measurements->TA2);
  nr_timer_stop(&l3_measurements->periodic_report_timer);

  l3_measurements->reports_sent = 0;
  l3_measurements->max_reports = 0;
  l3_measurements->report_interval_ms = 0;
}

static void handle_measobj_remove(rrcPerNB_t *rrc, struct NR_MeasObjectToRemoveList *remove_list, NR_UE_Timers_Constants_t *timers)
{
  // section 5.5.2.4 in 38.331
  for (int i = 0; i < remove_list->list.count; i++) {
    // for each measObjectId included in the received measObjectToRemoveList
    // that is part of measObjectList in the configuration
    NR_MeasObjectId_t id = *remove_list->list.array[i];
    if (id < 1 || id > MAX_MEAS_OBJ)
      continue;
    if (rrc->MeasObj[id]) {
      // remove the entry with the matching measObjectId from the measObjectList
      asn1cFreeStruc(asn_DEF_NR_MeasObjectToAddMod, rrc->MeasObj[id]);
      // remove all measId associated with this measObjectId from the measIdList
      for (int j = 1; j <= MAX_MEAS_ID; j++) {
        if (rrc->MeasId[j] && rrc->MeasId[j]->measObjectId == id) {
          asn1cFreeStruc(asn_DEF_NR_MeasIdToAddMod, rrc->MeasId[j]);
          handle_meas_reporting_remove(rrc, j, timers);
        }
      }
    }
  }
}

static void handle_quantityconfig(rrcPerNB_t *rrc, NR_QuantityConfig_t *quantityConfig, NR_UE_Timers_Constants_t *timers)
{
  if (quantityConfig->quantityConfigNR_List) {
    for (int i = 0; i < quantityConfig->quantityConfigNR_List->list.count && i < MAX_QUANTITY_CONFIG; i++) {
      NR_QuantityConfigNR_t *quantityNR = quantityConfig->quantityConfigNR_List->list.array[i];
      if (!rrc->QuantityConfig[i])
        rrc->QuantityConfig[i] = calloc(1, sizeof(*rrc->QuantityConfig[i]));
      // Transfer ownership; a shallow assignment leaves pointers into the
      // decoded DL message, which is freed after processing.
      UPDATE_NP_IE(rrc->QuantityConfig[i]->quantityConfigCell, quantityNR->quantityConfigCell, NR_QuantityConfigRS_t);
      // TODO: It remains to compute ssb_filter_coeff_rsrp and csi_RS_filter_coeff_rsrp for multiple quantityConfig
      // TS 38.331 - 5.5.3.2 Layer 3 filtering
      NR_QuantityConfigRS_t *qcc = &rrc->QuantityConfig[i]->quantityConfigCell;
      l3_measurements_t *l3_measurements = &rrc->l3_measurements;
      if (qcc->ssb_FilterConfig.filterCoefficientRSRP)
        l3_measurements->ssb_filter_coeff_rsrp = 1. / pow(2, (*qcc->ssb_FilterConfig.filterCoefficientRSRP) / 4);
      if (qcc->csi_RS_FilterConfig.filterCoefficientRSRP)
        l3_measurements->csi_RS_filter_coeff_rsrp = 1. / pow(2, (*qcc->csi_RS_FilterConfig.filterCoefficientRSRP) / 4);
      if (quantityNR->quantityConfigRS_Index)
        UPDATE_IE(rrc->QuantityConfig[i]->quantityConfigRS_Index, quantityNR->quantityConfigRS_Index, struct NR_QuantityConfigRS);
    }
  }
  for (int j = 1; j <= MAX_MEAS_ID; j++) {
    // for each measId included in the measIdList
    if (rrc->MeasId[j])
      handle_meas_reporting_remove(rrc, j, timers);
  }
}

static void handle_measid_remove(rrcPerNB_t *rrc, struct NR_MeasIdToRemoveList *remove_list, NR_UE_Timers_Constants_t *timers)
{
  for (int i = 0; i < remove_list->list.count; i++) {
    NR_MeasId_t id = *remove_list->list.array[i];
    if (id < 1 || id > MAX_MEAS_ID)
      continue;
    if (rrc->MeasId[id]) {
      asn1cFreeStruc(asn_DEF_NR_MeasIdToAddMod, rrc->MeasId[id]);
      handle_meas_reporting_remove(rrc, id, timers);
    }
  }
}

# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_executable(nr_ue_fuzz_hook_test nr_ue_fuzz_hook_test.c)
target_link_libraries(nr_ue_fuzz_hook_test PRIVATE asn1_nr_rrc asn1_lte_rrc_hdrs nr_common LOG minimal_lib)
add_dependencies(tests nr_ue_fuzz_hook_test)
add_test(NAME nr_ue_fuzz_hook_test COMMAND nr_ue_fuzz_hook_test)

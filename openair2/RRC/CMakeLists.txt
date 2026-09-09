# SPDX-License-Identifier: LicenseRef-CSSL-1.0

add_subdirectory(LTE)
add_subdirectory(NR)
if(ENABLE_TESTS)
  add_subdirectory(NR_UE/tests)
endif()

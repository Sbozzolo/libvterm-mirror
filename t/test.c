#include <stdio.h>
#include <stdlib.h>

#include <CUnit/Basic.h>

#include "externs.h"

static CU_SuiteInfo suites[] = {
#include "suites.h"
  CU_SUITE_INFO_NULL,
};

int main(int argc, char *argv[])
{
  setvbuf(stdout, NULL, _IONBF, 0);

  if(CU_initialize_registry() != CUE_SUCCESS) {
    fprintf(stderr, "Initialisation of test registry failed - %s\n", CU_get_error_msg());
    exit(1);
  }

  if(CU_register_suites(suites) != CUE_SUCCESS) {
    fprintf(stderr, "Registration of suites vailed - %s\n", CU_get_error_msg());
    exit(1);
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_set_error_action(CUEA_FAIL);

  int ret = CU_basic_run_tests();

  printf("Tests completed: result %d\n", ret);

  CU_cleanup_registry();

  return 0;
}

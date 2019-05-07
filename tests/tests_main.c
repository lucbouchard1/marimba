#include <stdio.h>

#include "CuTest.h"

CuSuite *StringGetSuite();
CuSuite *UtilsGetSuite();

void RunAllTests(void) {
   CuString *output = CuStringNew();
   CuSuite* suite = CuSuiteNew();

   CuSuiteAddSuite(suite, StringGetSuite());
   CuSuiteAddSuite(suite, UtilsGetSuite());

   CuSuiteRun(suite);
   CuSuiteSummary(suite, output);
   CuSuiteDetails(suite, output);
   printf("%s\n", output->buffer);
}

int main(void) {
   RunAllTests();
}
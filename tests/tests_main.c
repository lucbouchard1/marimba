#include <stdio.h>

#include "CuTest.h"

CuSuite *StringGetSuite();

void RunAllTests(void) {
   CuString *output = CuStringNew();
   CuSuite* suite = CuSuiteNew();

   CuSuiteAddSuite(suite, StringGetSuite());

   CuSuiteRun(suite);
   CuSuiteSummary(suite, output);
   CuSuiteDetails(suite, output);
   printf("%s\n", output->buffer);
}

int main(void) {
   RunAllTests();
}
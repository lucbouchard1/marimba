#include <string.h>

#include "CuTest.h"
#include "../src/string.h"

void Test_mmemset(CuTest *tc)
{
   char input[128], actual[128];

   memset(input, 24, 128);
   memset(actual, 0, 128);

   mmemset(input, 0, 128);
   CuAssertTrue(tc, !memcmp(input, actual, 128));
}

CuSuite *StringGetSuite()
{
   CuSuite *suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, Test_mmemset);
   return suite;
}
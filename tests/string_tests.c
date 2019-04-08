#include <string.h>
#include <stdio.h>

#include "CuTest.h"
#include "../src/string.h"

void Test_mmemset(CuTest *tc)
{
   char input[128], actual[128];

   memset(input, 24, 128);
   memset(actual, 0, 128);

   mmemset(input, 0, 128);
   CuAssertTrue(tc, !memcmp(input, actual, 128));

   memset(input, 13, 128);
   memset(actual, 10, 128);

   mmemset(input, 10, 128);
   CuAssertTrue(tc, !memcmp(input, actual, 128));

   memset(input, 13, 128);
   memset(actual, 0x12345, 128);

   mmemset(input, 0x12345, 128);
   CuAssertTrue(tc, !memcmp(input, actual, 128));
}

void Test_mmemcpy(CuTest *tc)
{
   char *actual = "This is a test string";
   char input[22];

   mmemcpy(input, actual, 22);
   CuAssertTrue(tc, !memcmp(input, actual, 22));

   memset(input, 0, 22);
   mmemcpy(input, actual, 15);
   CuAssertTrue(tc, !memcmp(input, actual, 15));
}

CuSuite *StringGetSuite()
{
   CuSuite *suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, Test_mmemset);
   SUITE_ADD_TEST(suite, Test_mmemcpy);
   return suite;
}
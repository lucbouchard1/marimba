#include "CuTest.h"
#include "../../src/string.h"

void Test_memset(CuTest *tc)
{
   char *actual = "aaaaaaaaaaaaaaa";
   char input[16];

   memset(input, 'a', 15);
   input[15] = 0;
   CuAssertStrEquals(tc, input, actual);
}

void Test_memcpy(CuTest *tc)
{
   char *actual = "This is a test string";
   char input[22];

   memcpy(input, actual, 22);
   CuAssertStrEquals(tc, input, actual);
}

void Test_strlen(CuTest *tc)
{
   char *str = "This is a test string";

   CuAssertIntEquals(tc, strlen(str), 21);
}

void Test_strcpy(CuTest *tc)
{
   char *actual = "This is a test string";
   char input[22];

   strcpy(input, actual);
   CuAssertStrEquals(tc, input, actual);
}

CuSuite *StringGetSuite()
{
   CuSuite *suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, Test_memset);
   SUITE_ADD_TEST(suite, Test_memcpy);
   SUITE_ADD_TEST(suite, Test_strlen);
   SUITE_ADD_TEST(suite, Test_strcpy);
   return suite;
}
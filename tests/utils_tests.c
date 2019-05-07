#include "CuTest.h"
#include "../src/utils.h"

struct FakeData {
   int data;
   int unused_data;
};

int fake_data_cmp(void *rarg1, void *rarg2)
{
   struct FakeData *arg1 = (struct FakeData *)rarg1;
   struct FakeData *arg2 = (struct FakeData *)rarg2;

   if (arg1->data > arg2->data)
      return -1;
   else if (arg2->data > arg1->data)
      return 1;
   else
      return 0;
}

void fake_data_swp(void *rarg1, void *rarg2)
{
   struct FakeData *arg1 = (struct FakeData *)rarg1;
   struct FakeData *arg2 = (struct FakeData *)rarg2, temp;

   temp = *arg2;
   *arg2 = *arg1;
   *arg1 = temp;
}

void Test_sort(CuTest *tc)
{
   int i;
   struct FakeData input[5] = {
      {.data = 3, .unused_data = 3},
      {.data = 6, .unused_data = 0},
      {.data = 1, .unused_data = 34},
      {.data = -2, .unused_data = 122},
      {.data = 10, .unused_data = -1029},
   };

   struct FakeData actual[5] = {
      {.data = -2, .unused_data = -40},
      {.data = 1, .unused_data = 25},
      {.data = 3, .unused_data = 111},
      {.data = 6, .unused_data = -220},
      {.data = 10, .unused_data = 11020},
   };

   sort(input, 5, sizeof(struct FakeData), fake_data_cmp, fake_data_swp);

   for (i = 0; i < 5; i++)
      CuAssertIntEquals(tc, input[i].data, actual[i].data);
}

CuSuite *UtilsGetSuite()
{
   CuSuite *suite = CuSuiteNew();
   SUITE_ADD_TEST(suite, Test_sort);
   return suite;
}
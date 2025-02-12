#include "harness/unity.h"
#include "../src/lab.h"


void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_stuff(void){
  // test stuff here
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_stuff);
  return UNITY_END();
}

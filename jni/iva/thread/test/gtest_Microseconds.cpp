#include "gtest_Microseconds.h"

MicrosecondsTest::MicrosecondsTest()
{   
};

MicrosecondsTest::~MicrosecondsTest() 
{
};

void MicrosecondsTest::SetUp() 
{
};

void MicrosecondsTest::TearDown() 
{
};

TEST_F(MicrosecondsTest, CreateMicrosecondsAndSleep) {

  Microseconds now;
  Microseconds(1000000).sleep();
  Microseconds now2;

  Microseconds diffti = now2 - now;
  
  EXPECT_TRUE(diffti.getTime() >= 999000);
  cout << diffti.getTime() << endl;

}

TEST_F(MicrosecondsTest, Comparison)
{
    ASSERT_TRUE(Microseconds(5) == 5);
    ASSERT_TRUE(Microseconds(5) >= 5);
    ASSERT_TRUE(Microseconds(5) >= 4);
    ASSERT_TRUE(Microseconds(5) <= 5);
    ASSERT_TRUE(Microseconds(5) <= 6);
    ASSERT_TRUE(Microseconds(5) > 4);
    ASSERT_TRUE(Microseconds(5) < 6);

    ASSERT_TRUE(Microseconds(5) == Microseconds(5));
    ASSERT_TRUE(Microseconds(5) >= Microseconds(5));
    ASSERT_TRUE(Microseconds(5) >= Microseconds(4));
    ASSERT_TRUE(Microseconds(5) <= Microseconds(5));
    ASSERT_TRUE(Microseconds(5) <= Microseconds(6));
    ASSERT_TRUE(Microseconds(5) > Microseconds(4));
    ASSERT_TRUE(Microseconds(5) < Microseconds(6));
}
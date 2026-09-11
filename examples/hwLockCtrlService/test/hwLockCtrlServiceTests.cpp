/// @brief  Tests for the HwLockCtrl::Service, demonstrating various unit
///         testing capabilities of the cms::test::sst_ctrl environment,
///         using cpputest.
/// @ingroup
/// @cond
///***************************************************************************
///
/// Contact Information:
///   Matthew Eshleman
///   https://covemountainsoftware.com
///   info@covemountainsoftware.com
///***************************************************************************
/// @endcond

#include <array>
#include <chrono>
#include <cassert>
#include "hwLockCtrl.h"
#include "hwLockCtrlService.hpp"
#include "cms_cpputest_sst_ctrl.hpp"
#include "bspTicks.hpp"

// the cpputest headers must always be last
#include "cms_assert_mock_support.hpp"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

static constexpr const char* HW_LOCK_CTRL_MOCK = "HwLockCtrl";

using namespace cms;
using namespace cms::test;

static std::array<SST::Evt const*, 10> testQueueStorage;

static int dummyContext = 1234;

static void TestSelfTestResultCallback(
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    HwLockCtrl::SelfTestResult result, HwLockCtrl::Service*, void* ctx)
{
    CHECK_EQUAL(&dummyContext, ctx);
    mock().actualCall("TestSelfTestResultCallback")
          .withUnsignedIntParameter("result",
                                    static_cast<unsigned int>(result));
}

/**
 * @brief These tests demonstrate the following key points:
 *         1) Does NOT test any thread that may be associated with the active
 *            object, rather, the associated cms::test::sst_ctrl environment is
 *            thread free, and "faked" to required the test to drive
 *            processing time.
 *         2) Tests the internal behavior of the active object without
 *            knowledge of the internal state machine. Rather, only
 *            by observing the behavior and associated output/results.
 *         3) Follow software engineering best practices, such
 *            as adhering to the DRY principle.
 *         4) Shows how to test behavior driven by SST timers (i.e. how
 *            to test the forward movement of time and expected behavior being
 *            tested.)
 */
TEST_GROUP(HwLockCtrlServiceTests) {
    std::unique_ptr<HwLockCtrl::Service> underTest = nullptr;

    void setup() final
    {
        using namespace cms::test;
        sst_ctrl::Setup(bsp::TICKS_PER_SECOND);
        underTest = std::make_unique<HwLockCtrl::Service>();
    }

    void teardown() final
    {
        mock().clear();
        sst_ctrl::Teardown();
    }

    void startServiceToLocked() const
    {
        underTest->RegisterSelfTestResultCallback(
            TestSelfTestResultCallback, &dummyContext);

        // setup mock to ensure the service under test calls the driver
        // API as expected.
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Init");
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");

        // start the active object under test.
        underTest->start(sst_ctrl::UNIT_UNDER_TEST_PRIORITY,
                         testQueueStorage.data(), testQueueStorage.size(),
                         nullptr);

        // give the system some processing time to handle
        // any internal or queued events
        giveProcessingTime();

        // check that the driver/mock interactions were all as expected.
        mock().checkExpectations();

        CHECK_TRUE(HwLockCtrl::Service::LockState::LOCKED == underTest->GetLockState());
    }

    void startServiceToUnlocked() const
    {
        startServiceToLocked();
        testUnlock();
    }

    void testUnlock() const
    {
        mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");
        underTest->UnlockAsync();
        giveProcessingTime();
        mock().checkExpectations();
        CHECK_TRUE(HwLockCtrl::Service::LockState::UNLOCKED == underTest->GetLockState());
    }

    static void giveProcessingTime()
    {
        sst_ctrl::ProcessEvents();
    }
};

TEST(HwLockCtrlServiceTests, given_init_when_created_then_does_not_crash)
{
    // setup() is automatically called by cpputest, which creates our unit under
    // test fully representing this trivial starting test.
    CHECK_TRUE(HwLockCtrl::Service::LockState::UNKNOWN == underTest->GetLockState());
}

TEST(HwLockCtrlServiceTests,
     given_startup_when_started_then_service_ensures_the_lock_is_locked)
{
    // When originally developed, this test contained all the code
    // in the below helper method. Since this "setup" was needed by
    // other tests, it was extracted into a helper method.
    // This pattern of developing tests, discovering the need for common
    // setup helper methods, takes place throughout coding of these unit tests,
    // and represents adhering to the DRY principle, even in our unit testing
    // code.
    startServiceToLocked();
}

TEST(HwLockCtrlServiceTests,
     given_locked_when_another_lock_request_then_service_is_silent)
{
    startServiceToLocked();
    underTest->LockAsync();
    giveProcessingTime();
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_locked_when_unlock_request_then_service_unlocks_the_driver)
{
    startServiceToLocked();
    testUnlock();
}

TEST(HwLockCtrlServiceTests,
     given_unlocked_when_another_unlock_request_then_service_is_silent)
{
    startServiceToUnlocked();
    underTest->UnlockAsync();
    giveProcessingTime();
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_unlocked_a_lock_request_will_return_to_locked)
{
    startServiceToUnlocked();
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");
    underTest->LockAsync();
    giveProcessingTime();
    mock().checkExpectations();
}

TEST(
    HwLockCtrlServiceTests,
    given_locked_when_selftest_requested_then_service_performs_selftest_publishes_results_and_returns_to_locked)
{
    startServiceToLocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_PASSED;

    //the self test call to the driver is first. setup to return passed.
    mock(HW_LOCK_CTRL_MOCK)
        .expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));

    //then we expect the callback to be hit, showing PASS
    mock().expectOneCall("TestSelfTestResultCallback")
          .withUnsignedIntParameter("result",
                                    static_cast<unsigned int>(
                                        HwLockCtrl::SelfTestResult::PASS));

    //then we expect the service to return to locked
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");

    underTest->DoSelfTestAsync();
    sst_ctrl::ProcessEvents();

    mock().checkExpectations();
}

TEST(
    HwLockCtrlServiceTests,
    given_unlocked_when_selftest_request_then_service_performs_selftest_emits_results_and_returns_to_unlocked)
{
    startServiceToUnlocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_PASSED;

    //the self test call to the driver is first. setup to return passed.
    mock(HW_LOCK_CTRL_MOCK)
        .expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));

    //then we expect the callback to be hit, showing PASS
    mock().expectOneCall("TestSelfTestResultCallback")
          .withUnsignedIntParameter("result",
                                    static_cast<unsigned int>(
                                        HwLockCtrl::SelfTestResult::PASS));

    //then we expect the service to return to locked
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");

    underTest->DoSelfTestAsync();
    sst_ctrl::ProcessEvents();

    mock().checkExpectations();
}

TEST(
    HwLockCtrlServiceTests,
    given_locked_when_selftest_request_which_fails_then_service_still_returns_to_locked)
{
    startServiceToLocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_FAILED_POWER;

    //the self test call to the driver is first. setup to return passed.
    mock(HW_LOCK_CTRL_MOCK)
        .expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));

    //then we expect the callback to be hit, showing PASS
    mock().expectOneCall("TestSelfTestResultCallback")
          .withUnsignedIntParameter("result",
                                    static_cast<unsigned int>(
                                        HwLockCtrl::SelfTestResult::FAIL));

    //then we expect the service to return to locked
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Lock");

    underTest->DoSelfTestAsync();
    sst_ctrl::ProcessEvents();

    mock().checkExpectations();
}

TEST(
    HwLockCtrlServiceTests,
    given_unlocked_when_selftest_request_which_fails_then_service_still_returns_to_unlocked)
{
    startServiceToUnlocked();

    auto passed = HW_LOCK_CTRL_SELF_TEST_FAILED_POWER;

    //the self test call to the driver is first. setup to return passed.
    mock(HW_LOCK_CTRL_MOCK)
        .expectOneCall("SelfTest")
        .withOutputParameterReturning("outResult", &passed, sizeof(passed));

    //then we expect the callback to be hit, showing PASS
    mock().expectOneCall("TestSelfTestResultCallback")
          .withUnsignedIntParameter("result",
                                    static_cast<unsigned int>(
                                        HwLockCtrl::SelfTestResult::FAIL));

    //then we expect the service to return to locked
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("Unlock");

    underTest->DoSelfTestAsync();
    sst_ctrl::ProcessEvents();

    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_locked_when_10secs_passes_then_service_polls_lock_comm_status)
{
    using namespace std::chrono_literals;

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("IsCommOk");
    sst_ctrl::MoveTimeForward(10s);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_locked_when_9900ms_passes_then_service_has_not_polled_yet)
{
    using namespace std::chrono_literals;

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectNoCall("IsCommOk");
    sst_ctrl::MoveTimeForward(9900ms);
    mock().checkExpectations();

    // while here, lets see if it calls the driver upon hitting the 10s mark.
    mock(HW_LOCK_CTRL_MOCK).expectOneCall("IsCommOk");
    sst_ctrl::MoveTimeForward(100ms);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_locked_when_60s_passes_then_service_has_polled_six_times)
{
    using namespace std::chrono_literals;

    // Demo multiple events over time, and confirm that the
    // service is polling in an ongoing manner.

    startServiceToLocked();
    mock(HW_LOCK_CTRL_MOCK).expectNCalls(6, "IsCommOk");
    sst_ctrl::MoveTimeForward(60s);
    mock().checkExpectations();
}

TEST(HwLockCtrlServiceTests,
     given_test_assert_event_will_assert_and_can_be_tested)
{
    static constexpr SST::Evt assertCausingEvent{HwLockCtrl::Service::DEMONSTRATE_TEST_OF_ASSERT};

    startServiceToLocked();

    cms::test::MockExpectDbcAssert();
    underTest->post(&assertCausingEvent);
    giveProcessingTime();
    mock().checkExpectations();
}
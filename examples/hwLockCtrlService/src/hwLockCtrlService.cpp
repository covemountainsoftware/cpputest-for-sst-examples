/// @brief A sample/demonstration QActive based service used
///        to demonstrate host based unit testing of an active object
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

#include "hwLockCtrlService.hpp"
#include "hwLockCtrl.h"
#include "bspTicks.hpp"
#include "dbc_assert.h"

DBC_MODULE_NAME("hwLockCtrlService");

namespace cms::HwLockCtrl {
// 10 second polling rate
static constexpr uint32_t TICKS_PER_POLL = bsp::TICKS_PER_SECOND * 10;

Service::Service()
    : SstFlatStateMachineTask()
      , m_history(Handled())
      , m_poll(POLL_COMM_STATUS, this)
      , m_lockState(LockState::UNKNOWN)
      , m_selfTestResultCb(nullptr)
      , m_selfTestResultCtx(nullptr)
{
}

Service::LockState Service::GetLockState() const
{
    return m_lockState;
}

void Service::RegisterSelfTestResultCallback(SelfTestResultCb cb, void* ctx)
{
    m_selfTestResultCb  = cb;
    m_selfTestResultCtx = ctx;
}

void Service::UnlockAsync()
{
    static constexpr SST::Evt ev{HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG};
    post(&ev);
}

void Service::LockAsync()
{
    static constexpr SST::Evt ev{HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG};
    post(&ev);
}

void Service::DoSelfTestAsync()
{
    static constexpr SST::Evt ev{HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG};
    post(&ev);
}

FlatStateMachine<SST::Evt>::StateRtn
Service::InitialPseudoState(const SST::Evt*)
{
    HwLockCtrlInit();
    m_poll.arm(TICKS_PER_POLL, TICKS_PER_POLL);
    return TransitionTo(&Service::StateOfLocked);
}

FlatStateMachine<SST::Evt>::StateRtn Service::StateOfLocked(const SST::Evt* e)
{
    switch (e->sig) {
        case SM_ENTER_SIG:
            HwLockCtrlLock();
            notifyChangedState(LockState::LOCKED);
            return Handled();

        case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG: {
            return Handled();
        }
        case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG: {
            return TransitionTo(&Service::StateOfUnlocked);
        }
        case POLL_COMM_STATUS: {
            // this is just to demonstrate unit testing of time
            // based functionality.
            HwLockCtrlIsCommOk();
            return Handled();
        }
        case HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG:
            return TransitionTo(&Service::StateOfSelfTest);
        default: {
            return Handled();
        }
    }
}

FlatStateMachine<SST::Evt>::StateRtn Service::StateOfUnlocked(const SST::Evt* e)
{
    switch (e->sig) {
        case SM_ENTER_SIG: {
            HwLockCtrlUnlock();
            notifyChangedState(LockState::UNLOCKED);
            return Handled();
        }
        case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG: {
            return Handled();
        }
        case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG: {
            return TransitionTo(&Service::StateOfLocked);
        }
        case POLL_COMM_STATUS: {
            // this is just to demonstrate unit testing of time
            // based functionality.
            HwLockCtrlIsCommOk();
            return Handled();
        }
        case HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG: {
            return TransitionTo(&Service::StateOfSelfTest);
        }
        default: {
            return Handled();
        }
    }
}

FlatStateMachine<SST::Evt>::StateRtn Service::StateOfSelfTest(const SST::Evt* e)
{
    switch (e->sig) {
        case SM_ENTER_SIG: {
            performSelfTest();
            return Handled();
        }
        case REQUEST_GOTO_HISTORY: {
            return m_history;
        }
        default: {
            return Handled();
        }
    }
}

void Service::performSelfTest()
{
    HwLockCtrlSelfTestResult result;
    const bool ok = HwLockCtrlSelfTest(&result);
    if (ok && (result == HW_LOCK_CTRL_SELF_TEST_PASSED)) {
        notifySelfTestResult(SelfTestResult::PASS);
    }
    else {
        notifySelfTestResult(SelfTestResult::FAIL);
    }

    // remind self to transition back to
    // history per this service's requirements
    //
    //  https://covemountainsoftware.com/2020/03/08/uml-statechart-handling-errors-when-entering-a-state/
    //
    // ideally we should add a postLIFO to SST. It doesn't currently exist.
    //
    static constexpr SST::Evt event{REQUEST_GOTO_HISTORY};
    post(&event);
}

void Service::notifyChangedState(const LockState state)
{
    m_lockState = state;

    switch (state) {
        case LockState::LOCKED:
            m_history = TransitionTo(&Service::StateOfLocked);
            break;
        case LockState::UNLOCKED:
            m_history = TransitionTo(&Service::StateOfUnlocked);
            break;
        default:
            DBC_ASSERT(__LINE__, true == false);
            break;
    }
}

void Service::notifySelfTestResult(const SelfTestResult result)
{
    //there is no generic publish concept with SST, so instead
    //we are using a callback approach.
    switch (result) {
        case SelfTestResult::PASS: // fall through on purpose
        case SelfTestResult::FAIL:
            if (m_selfTestResultCb != nullptr) {
                m_selfTestResultCb(result, this, m_selfTestResultCtx);
            }
            break;
        default:
            DBC_ERROR(__LINE__);
            break;
    }
}
} // namespace cms::HwLockCtrl
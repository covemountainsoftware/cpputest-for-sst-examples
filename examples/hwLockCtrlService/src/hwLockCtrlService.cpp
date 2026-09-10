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
    , m_poll(POLL_COMM_STATUS, this)
    , m_lockState(LockState::UNKNOWN)
{
}

Service::LockState Service::GetLockState() const
{
    return m_lockState;
}

void Service::UnlockAsync()
{
    static constexpr SST::Evt ev {HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG};
    post(&ev);
}

void Service::LockAsync()
{
    static constexpr SST::Evt ev {HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG};
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
        case SM_ENTER:
            HwLockCtrlLock();
            notifyChangedState(LockState::LOCKED);
            return Handled();
        case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG:
            return Handled();
        case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG:
            return TransitionTo(&Service::StateOfUnlocked);
        case POLL_COMM_STATUS:
            // this is just to demonstrate unit testing of time
            // based functionality.
            HwLockCtrlIsCommOk();
            return Handled();
        default:
            return Handled();
    }
}

FlatStateMachine<SST::Evt>::StateRtn Service::StateOfUnlocked(const SST::Evt* e)
{
    switch (e->sig) {
        case SM_ENTER:
            HwLockCtrlUnlock();
            notifyChangedState(LockState::UNLOCKED);
            return Handled();
        case HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG:
            return Handled();
        case HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG:
            return TransitionTo(&Service::StateOfLocked);
        case POLL_COMM_STATUS:
            // this is just to demonstrate unit testing of time
            // based functionality.
            HwLockCtrlIsCommOk();
            return Handled();
        default:
            return Handled();
    }
}

FlatStateMachine<SST::Evt>::StateRtn Service::StateOfSelfTest(const SST::Evt* e)
{
    (void)e;   // todo
    return Handled();
}

void Service::performSelfTest()
{
    HwLockCtrlSelfTestResult result;
    bool ok = HwLockCtrlSelfTest(&result);
    if (ok && (result == HW_LOCK_CTRL_SELF_TEST_PASSED)) {
        notifySelfTestResult(SelfTestResult::PASS);
    }
    else {
        notifySelfTestResult(SelfTestResult::FAIL);
    }

    // remind self to transition back to
    // history per this service's requirements
    // note the use of "postLIFO" (urgent) as per:
    //
    //  https://covemountainsoftware.com/2020/03/08/uml-statechart-handling-errors-when-entering-a-state/
    //
    // TODO static constexpr SST::Evt event {REQUEST_GOTO_HISTORY};

    // TODO post(&event);
}

void Service::notifyChangedState(const LockState state)
{
    m_lockState = state;

    // TODO
    /*
    static const QP::QEvt lockedEvent =
      QP::QEvt(HW_LOCK_CTRL_SERVICE_IS_LOCKED_SIG);
    static const QP::QEvt unlockedEvent =
      QP::QEvt(HW_LOCK_CTRL_SERVICE_IS_UNLOCKED_SIG);

    switch (state) {
        case LockState::LOCKED:
            QP::QF::PUBLISH(&lockedEvent, this);
            m_history = &locked;
            break;
        case LockState::UNLOCKED:
            QP::QF::PUBLISH(&unlockedEvent, this);
            m_history = &unlocked;
            break;
        default:
            Q_ASSERT(true == false);
            break;
    }
    */
}

void Service::notifySelfTestResult(const SelfTestResult result)
{
    (void)result;
    /* TODO
    switch (result) {
        case SelfTestResult::PASS:   // fall through on purpose
        case SelfTestResult::FAIL:
            SelfTestEvent::publish<HW_LOCK_CTRL_SERVICE_SELF_TEST_RESULTS_SIG>(
              result);
            break;
        default:
            Q_ASSERT(true == false);
            break;
    }
    */
}

}   // namespace cms::HwLockCtrl

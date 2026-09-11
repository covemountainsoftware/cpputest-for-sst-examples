/// @file hwLockCtrlService.hpp
/// @brief A service (active object) for demonstrating cpputest host
///        based unit testing in an SST environment.
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

#ifndef DEMO_SST_HW_LOCK_CTRL_SERVICE_HPP
#define DEMO_SST_HW_LOCK_CTRL_SERVICE_HPP

#include "sst.hpp"
#include "cms_sst_flat_state_machine_task.hpp"
#include "hwLockCtrlSelfTestResultEnum.hpp"
#include <atomic>

namespace cms::HwLockCtrl {

/**
 * @brief the HwLockCtrl::Service demonstration active object provides for
 *        higher level hardware lock control behavior.
 *        For example, this service will automatically return the hardware lock
 *        to its last state after completing a self test request.
 */
class Service : public SstFlatStateMachineTask {
public:

    using SelfTestResultCb = void (*)(
        SelfTestResult result, Service* service, void* ctx);

    enum class LockState { UNKNOWN, LOCKED, UNLOCKED };

    enum DirectSignals {
        PING = SM_BEGIN_USER_SIGNALS,
        HW_LOCK_CTRL_SERVICE_REQUEST_LOCKED_SIG,
        HW_LOCK_CTRL_SERVICE_REQUEST_UNLOCKED_SIG,
        HW_LOCK_CTRL_SERVICE_REQUEST_SELF_TEST_SIG,
        DEMONSTRATE_TEST_OF_ASSERT,
        MAX_DIRECT_SIG
    };

    Service();
    ~Service() override = default;

    Service(const Service&)            = delete;
    Service& operator=(const Service&) = delete;
    Service(Service&&)                 = delete;
    Service& operator=(Service&&)      = delete;

    /**
     * @return the last known lock state of this service
     */
    [[nodiscard]] LockState GetLockState() const;

    /**
     * @param cb a callback to execute upon completing any self test request
     * @param ctx a void* context pointer to provide with the callback.
     * @note it is generally best from a concurrency point of view to ensure this
     * has been called before starting this active object. Or at least call it
     * before requesting a self test.
     */
    void RegisterSelfTestResultCallback(SelfTestResultCb cb, void* ctx);

    /**
     * Send an event to this active object to
     * attempt to unlock.
     */
    void UnlockAsync();

    /**
     * Send an event to this active object to
     * attempt to lock.
     */
    void LockAsync();

    /**
     * Send an event to this active object to perform its internal
     * self test actions, then return to its current state.
     */
    void DoSelfTestAsync();

protected:
    enum InternalSignals {
        REQUEST_GOTO_HISTORY = MAX_DIRECT_SIG,
        POLL_COMM_STATUS
    };

    StateRtn InitialPseudoState(const SST::Evt* event) override;

    StateRtn StateOfLocked(const SST::Evt* e);
    StateRtn StateOfUnlocked(const SST::Evt* e);
    StateRtn StateOfSelfTest(const SST::Evt* e);

private:
    void notifySelfTestResult(SelfTestResult result);
    void notifyChangedState(LockState state);
    void performSelfTest();

    StateRtn m_history;
    SST::TimeEvt m_poll;
    std::atomic<LockState> m_lockState;
    SelfTestResultCb m_selfTestResultCb;
    void*            m_selfTestResultCtx;
};
}   // namespace cms::HwLockCtrl

#endif   // DEMO_SST_HW_LOCK_CTRL_SERVICE_HPP

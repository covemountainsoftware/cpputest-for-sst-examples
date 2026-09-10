/**
 *    This is a copy of the cpputest-for-sst port file
 *    being used for this example project only.
 *    In an actual target microcontroller build, instead
 *    use the appropriate port from the sst itself.
 */
#ifndef CPPUTEST_FOR_SST_EXAMPLES_SST_PORT_HPP
#define CPPUTEST_FOR_SST_EXAMPLES_SST_PORT_HPP

#include <cstdint>

#define SST_PORT_MAX_TASK 32U

// additional SST-PORT task attributes
#define SST_PORT_TASK_ATTR  SST::TaskPrio m_prio;

// SST-PORT disabling/enabling interrupts
#define SST_PORT_INT_DISABLE() do{}while(0)
#define SST_PORT_INT_ENABLE()  do{}while(0)

// SST-PORT critical section
#define SST_PORT_CRIT_STAT
#define SST_PORT_CRIT_ENTRY() do{}while(0)
#define SST_PORT_CRIT_EXIT()  do{}while(0)

namespace SST {
using ReadySet = std::uint32_t;

//! SST lock key
using LockKey = std::uint32_t;

// special idle callback to handle the "idle condition" in SST0
void onIdleCond();

}

#define SST_LOG2(x_) (static_cast<std::uint_fast8_t>(32U - __builtin_clz((unsigned)(x_))))

#define SST_PORT_TASK_OPER static void runUntilNoReadyActiveObjects();

#endif   // CPPUTEST_FOR_SST_EXAMPLES_SST_PORT_HPP

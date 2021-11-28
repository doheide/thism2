// It is generally recommended to have a single file provide the main
// of a testing binary, and other test files to link against it.

// Let Catch provide main():
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "sm_sys.h"
// That's it

SMSys *smsys = 0;
// Compile implementation of Catch for use with files that do contain tests:
// - g++ -std=c++11 -Wall -I$(CATCH_SINGLE_INCLUDE) -c 000-CatchMain.cpp
// - cl -EHsc -I%CATCH_SINGLE_INCLUDE% -c 000-CatchMain.cpp

TEST_CASE( "Test ok", "[Base]" ) {
    REQUIRE(1 == 1);
}


SCENARIO("setup of state machines system", "[tsmsys]" ) {

    GIVEN("State machine system is prepared for basic tests") {
        HWAL_Std_No_HW hwal(HWAL_Log::Debug);
        SMSys csmsys(&hwal);
        smsys = &csmsys;

        WHEN("system is instantiated") {
            THEN("the system is correctly presetup") {
                REQUIRE(smsys->numberOfStatesGet() == 7);
                REQUIRE(smsys->maxLevelGet() == 2);
            }
        }WHEN("system is initialized") {
            smsys->initialSetup();
            THEN("initial states are active") {
                REQUIRE(smsys->isStateActive<S_Off>());
                REQUIRE(smsys->isStateActive<S_LED_Off>());
            }THEN("non-initial states are not active") {
                REQUIRE(!smsys->isStateActive<S_LED_On>());
                REQUIRE(!smsys->isStateActive<S_On>());
                REQUIRE(!smsys->isStateActive<S_Blink>());
                REQUIRE(!smsys->isStateActive<S_Blink_On>());
                REQUIRE(!smsys->isStateActive<S_Blink_Off>());
            }THEN("two initialize events and E_LED_off are in the queue") {
                REQUIRE(smsys->eventsInBuffer() == 3);
            }THEN("after processEvents() queue is empty") {
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 0);
            }
        }
    }

    GIVEN("State machine system is prepared for LED_On") {
        HWAL_Std_No_HW hwal(HWAL_Log::Debug);
        SMSys csmsys(&hwal);
        smsys = &csmsys;

        WHEN("system is initialized and event E_On is raised") {
            smsys->raiseEvent<E_On>();

            THEN("one event is in the event queue") {
                REQUIRE(smsys->eventsInBuffer() == 1);
            }
            THEN("after processEvents() state S_On is active, S_Off is inactive and E_LED_on and an initial "
                 "event is in the event queue") {
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 2);
                REQUIRE(smsys->isStateActive<S_On>() == true);
                REQUIRE(smsys->isStateActive<S_Off>() == false);
            }
            THEN("after two processEvents() state S_LED_On is active, S_LED_Off is inactive and E_LED_on and an initial "
                 "event is in the event queue") {
                smsys->processEvents();
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 1);
                REQUIRE(smsys->isStateActive<S_LED_On>()==true);
                REQUIRE(smsys->isStateActive<S_LED_Off>()==false);
            }
            THEN("after three processEvents() no is in the event queue") {
                smsys->processEvents();
                smsys->processEvents();
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 0);
            }
        }
        WHEN("system is in state S_On (and S_LED_On) and ") {
            smsys->raiseEvent<E_On>();
            for(int i=0;i!=3;i++)
                smsys->processEvents();
            smsys->raiseEvent<E_Off>();

            THEN("one event is in the event queue") {
                REQUIRE(smsys->eventsInBuffer() == 1);
            }
            THEN("after processEvents() state S_Off is active, S_On is inactive and E_LED_on and an initial event is in the event queue") {
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 2);
                REQUIRE(smsys->isStateActive<S_On>() == false);
                REQUIRE(smsys->isStateActive<S_Off>() == true);
            }
            THEN("after two processEvents() state S_LED_Off is active, S_LED_On is inactive and E_LED_off and an initial event is in the event queue") {
                smsys->processEvents();
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 1);
                REQUIRE(smsys->isStateActive<S_LED_On>()==false);
                REQUIRE(smsys->isStateActive<S_LED_Off>()==true);
            }
            THEN("after three processEvents() no is in the event queue and the initial states of all state machines are active") {
                smsys->processEvents();
                smsys->processEvents();
                smsys->processEvents();
                REQUIRE(smsys->eventsInBuffer() == 0);

                REQUIRE(smsys->isStateActive<S_Off>());
                REQUIRE(smsys->isStateActive<S_LED_Off>());

                REQUIRE(!smsys->isStateActive<S_LED_On>());
                REQUIRE(!smsys->isStateActive<S_On>());
                REQUIRE(!smsys->isStateActive<S_Blink>());
                REQUIRE(!smsys->isStateActive<S_Blink_On>());
                REQUIRE(!smsys->isStateActive<S_Blink_Off>());
            }

        }

    }
}




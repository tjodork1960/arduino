//******************************************************************************************
//  File: IS_ShadeControl.h
//  Authors: Tim O'Callaghan based on Is_DoorControl.h
//
//  Summary:  IS_ShadeControl is a class which implements the SmartThings "Shade" device capability.  It features
//			  motor control and the use of a timer and optional switch for both open and close.  When a switch is used the timer should be set so that in normal operation the switch will trip before the timer.
//			  The input will be handled by interrupts.  There is 2 outputs to control direction and a PWM 'analog' output to control speed.
//
//			  It inherits from the st::InterruptSensor class and clones much from the st::Executor Class
//
//			  Create an instance of this class in your sketch's global variable section
//			  For Example:  st::IS_ShadeControl sensor3(F("shade1"), PIN_OPEN_SWITCH, 10000,0,14000,LOW, true, PIN_MOTOR_OPEN,PIN_MOTOR_CLOSED,PIN_MOTORENABLE, 500,open, false);
//
//			  st::IS_ShadeControl() constructor requires the following arguments
//				- String &name - REQUIRED - the name of the object - must match the Groovy ST_Anything DeviceType tile name
//				- byte pinOpenSW - REQUIRED - the Arduino Pin to be used as a digital input for open switch   (if not used set to 0, then timer will stop for opening)
//				- long openTimeLimit - REQUIRED - the number of milliseconds to run shade to open if no switch encountered
//				- byte pinClosedSW - REQUIRED - the Arduino Pin to be used as a digital input for closed switch  (if not used set to 0, then time will stop down)
//				- long closedTimeLimit - REQUIRED - the number of milliseconds to run shade to close if no switch encountered
//				- bool interruptActiveState - REQUIRED - LOW or HIGH - determines which value indicates the interrupt is true for both inputs
//				- bool internalPullup - REQUIRED - true == INTERNAL_PULLUP for both inputs
//				- byte pinOutputOpen - REQUIRED - the Arduino Pin to be used as a digital output for open
//				- byte pinOutputClose - REQUIRED - the Arduino Pin to be used as a digital output for close
//				- byte pinMotorEnablePWM - REQUIRED - the Arduino Pin to be used as a digital PWM output -simulate analog
//              - long PWMSpeedValue to output on PWM enable output
//				- bool desiredStartState - REQUIRED - the value desired for the initial state of the switch.  (open or closed)
//				- bool invertOutputLogic - REQUIRED - determines whether the Arduino Digital Outputs should use inverted logic
//
// Note: Motor controller is   L298N Dual Motor Controller Module 


//  Change History:
//
//    Date        Who            What
//    ----        ---            ----
//    2020-12-31  Tim OC         Original Creation
//
//
//******************************************************************************************

#ifndef ST_IS_SHADECONTROL_H
#define ST_IS_SHADECONTROL_H

#include "InterruptSensor.h"
enum state {open=1,opening=2,closed=3,closing=4};
enum command {Open=1,Close=2,Stop=3};

namespace st
{
	class IS_ShadeControl : public InterruptSensor
	{
		private:
			//inherits everything necessary from parent InterruptSensor Class for the Contact Sensor

			//following are for the digital inputs
			byte m_nPinSWOpened;                                        //set switch to 0 if no switch
			byte m_nPinSWClosed;                                        //set switch to 0 if no switch
			unsigned long m_lOpenTimeLimit;
			unsigned long m_lClosedTimeLimit;
			bool m_bInternalPullup;
            bool m_bInterruptActiveState;

    		//following are for the digital outputs
     		byte m_npinMotorOutputOpen;
			byte m_npinMotorOutputClose;
            byte m_npinMotorOutputEnablePWM;
			unsigned long m_lMotorPWMSpeed;
			bool m_bInvertLogic;	//determines whether the Arduino Digital Output should use inverted logic

			//current state open,opening,closed,closing,unknown
			state m_eCurrentState;	
			
			bool m_bTimerPending;		//true if waiting on timer to expire
            state m_eDesiredStartingState;
			void controlMotor();	//function to Open, Close, Stop
			
		public:
			//constructor - momentary output - called in your sketch's global variable declaration section
        	IS_Shade(const __FlashStringHelper *name, byte pinOpenSW,unsigned long openTimeLimit,byte pinClosedSW,long closedTimeLimit,  bool interruptActiveState, bool internalPullup, byte pinMotorOutputForward,byte pinMotorOutputReverse, byte pinMotorEnablePWM, unsigned long PWMSpeedValue, state desiredStartingState=up, bool invertOutputLogic);
			
			//destructor
			virtual ~IS_ShadeControl();
			
			//initialization function
			virtual void init();

			//update function 
			void update();

			//SmartThings Shield data handler (receives command to turn "Open, Close or Stop the motor  (digital output)
			virtual void beSmart(const String &str);

			//called periodically by Everything class to ensure ST Cloud is kept consistent with the state of the contact sensor
			virtual void refresh();

			//handles what to do when interrupt is triggered 
			virtual void runInterrupt();

			//handles what to do when interrupt is ended 
			virtual void runInterruptEnded();

			//gets
			virtual byte getPin() const { return m_nOutputPin; }

			//sets
			virtual void controlMotor(command c);
	};
}


#endif

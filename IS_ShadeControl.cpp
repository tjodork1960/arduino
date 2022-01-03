//******************************************************************************************
//  File: IS_ShadeControl.cpp
//  Authors: Tim O'Callaghan
//
//  See .h for details
//
//  Change History:
//
//    Date        Who            What
//    ----        ---            ----
//    2020-12-31  Tim OCallaghan
/
//
//******************************************************************************************

#include "IS_ShadeControl.h"

#include "Constants.h"
#include "Everything.h"


# open always means move shade to let light in
# close always means move shade to block light


namespace st
{
//private
    //valid commands are Stop, Open and Close

	void IS_ShadeControl::controlMotor(command c) 
	{ 
	   if ((c == Stop) && (( m_eCurrentState == opening) || (m_eCurrentState == closing))) {
		     digitalWrite(m_npinMotorOutputOpen,  m_bInvertLogic ? LOW: HIGH);
			 digitalWrite(m_npinMotorOutputClose,  m_bInvertLogic ? LOW: HIGH);
             analogWrite(m_npinMotorOutputEnablePWM, 0);
			 m_eCurrentState = unknown;

 			//cancel timer
			CancelTimer();
			 
	   } else if (c == Open) {
		     digitalWrite(m_npinMotorOutputOpen,  m_bInvertLogic ? HIGH : LOW);
			 digitalWrite(m_npinMotorOutputClose,  m_bInvertLogic ? LOW : HIGH);
             analogWrite(m_npinMotorOutputEnablePWM, m_lMotorPWMSpeed); 	

     		 //Save time operation limit
             m_lTimeOperationDone = millis() + 	m_lOpenTimeLimit;		 

     		 //Increment number of active timers
     		 st::Everything::bTimersPending++;
			 m_bTimerPending = true;
			
			 m_bCurrentState = OPENING;	

	   } else if (c == Close) {
		     digitalWrite(m_npinMotorOutputOpen,  m_bInvertLogic ? LOW : HIGH);
			 digitalWrite(m_npinMotorOutputClose,  m_bInvertLogic ? HIGH : LOW);
             analogWrite(m_npinMotorOutputEnablePWM, m_lMotorPWMSpeed); 	

			//Save time operation limit
			m_lTimeOperationDone = millis() + 	m_lClosedTimeLimit;	

    		//Increment number of active timers
     		st::Everything::bTimersPending++;
			m_bTimerPending = true;		

			m_bCurrentState = CLOSING;		

       }
	}

//public
	//constructor

			
		public:
			//constructor - called in your sketch's global variable declaration section
	IS_ShadeControl(const __FlashStringHelper *name, byte pinSWOpen,unsigned long openTimeLimit,byte pinSWClosed,long closedTimeLimit,  bool interruptActiveState, bool internalPullup, byte pinOutputOpen,byte pinOutputClose, byte pinMotorEnablePWM, unsigned long PWMSpeedValue, state desiredStartingState=closed, bool invertOutputLogic):
		InterruptSensor(name, pinSWOpen, iState, internalPullup, 1 ),  //use parent class' constructor
		m_nPinSWOpened(pinSWOpen),
		m_lOpenTimeLimit(openTimeLimit),
		m_nPinSWClosed(pinSWClosed),
		m_lCloseTimeLimit(closedTimeLimit),
		m_bInterruptActiveState(interruptActiveState),
		m_bInternalPullup(internalPullup),
		m_npinMotorOutputOpen(pinOutputOpen),
		m_npinMotorOutputClose(pinOutputClose),
        m_npinMotorOutputEnablePWM(pinMotorEnablePWM),
		m_lMotorPWMSpeed(PWMSpeedValue),
		m_eDesiredStartingState(desiredStartingState),
		m_bInvertLogic(invertOutputLogic),
		m_lTimeOperationDone(0),
		m_bTimerPending(false),
		m_eCurrentState(unknown),
		{
			if ((m_eDesiredStartingState == open) && (m_eCurrentState != open)) {
				controlMotor(open);
			} else if ((m_eDesiredStartingState == closed) && (m_eCurrentState != closed)) {
				controlMotor(close);
			} 	
				
		}
	
	//destructor
	IS_ShadeControl::~IS_ShadeControl()
	{
	}
	
	void IS_ShadeControl::init()
	{
		//get current status of open and closed sensors by calling parent class's init() routine - no need to duplicate it here!
		InterruptSensor::init();
	}



	//update function 
	void IS_ShadeControl::update()

         //check to see if input pin has changed state  -does this handle both input switches ?
		InterruptSensor::update();  
		
		//check  if open switch defined and just opened or closed switch defined and just closed
		if (   ((m_nPinSWOpened != 0) && Contact(m_nPinSWOpened) && (m_eCurrentState == opening)) || 
		       ((m_nPinSWClosed != 0) && Contact(m_nPinSWClosed) && (m_eCurrentState == closing)) )  {

			//stop motor
			controlMotor(stop);

			//update state
			m_eCurrentState = (m_eCurrentState == closing)?closed:open;
        }

		//Timer has expired and was opening or closing 
		if (m_bTimerPending) {
			if ( ((m_eCurrentState == opening) || (m_eCurrentState == closing)) && (millis() > m_lTimeOperationDone))
			{		
				//stop motor
				controlMotor(stop);
				
				//if hit time limit and no switch assume that it worked else set to unknown
				if (m_eCurrentState == opening) {
					if (m_nPinOpenedSW ==0)  {
						m_eCurrentState = open;
				    } else {
     					m_eCurrentState = unknown;
					}	
                } else if (m_eCurrentState == closing) {
					if (m_nPinClosedSW ==0)  {
						m_eCurrentState = closed;
				    } else {
     					m_eCurrentState = unknown;
					}	
				}
				
				//Decrement number of active timers
				if (st::Everything::bTimersPending > 0) st::Everything::bTimersPending--;
				m_bTimerPending = false;
			}
		}
		
		//check to see if input pin has changed state
		//InterruptSensor::update();  moved to top
	}

//on = open   off=closed ==> switched to use open and close ---is that ok ?
	void IS_ShadeControl::beSmart(const String &str)
	{
		String s = str.substring(str.indexOf(' ') + 1);
		if (st::InterruptSensor::debug) {
			Serial.print(F("IS_ShadeControl::beSmart s = "));
			Serial.println(s);
		}

		if (s == F("open"))
		{
			if ((m_eCurrentState == closed) && (!m_bTimerPending)) {
				
     			controlMotor(open);		
   
	    		//Queue the door status update the ST Cloud 
		    	Everything::sendSmartStringNow(getName() +  F(" opening") );					
			}
			
		}  else if (s == F("close")) {
			
			if ((m_eCurrentState == open) && (!m_bTimerPending)) {
				
     			controlMotor(close);	

			   //Queue the door status update the ST Cloud 
			   Everything::sendSmartStringNow(getName() +  F(" closing") );				
			}

		}
		
	}


	//called periodically by Everything class to ensure ST Cloud is kept consistent with the state of the contact sensor
	void IS_ShadeControl::refresh()
	{
		Everything::sendSmartString(getName() + (getStatus() ? F(" closed") : F(" open")));   //how does getStatus work ????
	}

    void IS_ShadeControl::CancelTimer()
	{
		if (st::Everything::bTimersPending > 0) st::Everything::bTimersPending--;
		m_bTimerPending = false;
	}

	void IS_ShadeControl::runInterrupt()                                                      //how does this work for 2 switches (open and closed)
	{
		//add the "closed" event to the buffer to be queued for transfer to the ST Shield
		Everything::sendSmartString(getName() + F(" closed"));
	}
	
	void IS_ShadeControl::runInterruptEnded()                                                  //how does this work for 2 switches (open and closed)
	{
		//add the "open" event to the buffer to be queued for transfer to the ST Shield
		Everything::sendSmartString(getName() + F(" open"));
	}

}





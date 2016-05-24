#include "PietteTech_DHT/PietteTech_DHT.h"

#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D2         	    // Digital pin for communications
#define LDRPIN   A0                 // Analog pin for Light reading

#define WATCHDOG_TIMEOUT 10000  //10 000 ms for watchdog to reset the device in case of cloud failure
#define INTERVAL 600
#define EVENT_NAME "test_DHT"

void dht_wrapper(); // must be declared before the lib initialization
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

bool eventPending;      // signal for receiving feedback from cloud on event we sent
bool safeModePending;      // signal for receiving feedback from cloud on event we sent


// Handler for cloud event
void eventHandler(const char *event, const char *data)
{
    eventPending = false;
}


// Cloud function to send device to Safe Mode for user firmware upgrade
int gotoSafeMode(String command){
    safeModePending = true;
//    System.enterSafeMode();
    return 1;
}


void setup()
{
    Particle.subscribe(EVENT_NAME, eventHandler);   // set up validaton mechanism
    eventPending = true;                            // initialize flag for it
    Particle.function("safe", gotoSafeMode);        // set up cloud function for DFU
}


// This wrapper is in charge of calling, must be defined like this for the lib work
void dht_wrapper() {
    DHT.isrCallback();
}


void loop()
{
    DHT.acquire();                  // start data acquisition
    
    while(DHT.acquiring()){         // wait here until data avail
        Particle.process();         // give system thread some priority while we wait
    }

    int result = DHT.getStatus();   // get DHT status

    if(!Particle.connected()){      // maybe not needed, but to be on safe side
        RGB.color(255,0,0);         // show user red light indicator on no-connection state
        Particle.connect();             // reconnect to cloud
        Particle.process();         // this probably should never happen, maybe..
    }

    RGB.control(true);              // take over rgb led control
    RGB.color(0, 255, 0);           // show user green light indicator on ok state

    switch (result) {
		case DHTLIB_OK:
            Particle.publish(EVENT_NAME, String::format("{\"H\" : %3.2f,   \"T\" : %3.2f,  \"L\" : %d }",
                             DHT.getHumidity(), DHT.getCelsius(), 4096-analogRead(LDRPIN)));
		    break;
		case DHTLIB_ERROR_CHECKSUM:
		    Particle.publish(EVENT_NAME,"Error\n\r\tChecksum error");
		    break;
		case DHTLIB_ERROR_ISR_TIMEOUT:
		    Particle.publish(EVENT_NAME,"Error\n\r\tISR time out error");
		    break;
		case DHTLIB_ERROR_RESPONSE_TIMEOUT:
		    Particle.publish(EVENT_NAME,"Error\n\r\tResponse time out error");
		    break;
		case DHTLIB_ERROR_DATA_TIMEOUT:
		    Particle.publish(EVENT_NAME,"Error\n\r\tData time out error");
		    break;
		case DHTLIB_ERROR_ACQUIRING:
		    Particle.publish(EVENT_NAME,"Error\n\r\tAcquiring");
		    break;
		case DHTLIB_ERROR_DELTA:
		    Particle.publish(EVENT_NAME,"Error\n\r\tDelta time to small");
		    break;
		case DHTLIB_ERROR_NOTSTARTED:
		    Particle.publish(EVENT_NAME,"Error\n\r\tNot started");
		    break;
		default:
		    Particle.publish(EVENT_NAME,"Unknown error");
		    break;
    }

    long watchDogTimer = millis(); // establish watchdog to verify cloud got the data
    while(eventPending){              // wait for signal that we received the event sent
        RGB.color(255,255,0);         // show yellow indicator on waiting
        Particle.process();         // while waiting, give OS time to process
        if(millis() > watchDogTimer+WATCHDOG_TIMEOUT){ // watchdog resets system if takes too long
                System.reset();
        }
    }
    
    RGB.color(0,0,255);
    long extraDelay = millis()+2000;  //allow couple extra second for processing 
    while(millis()<extraDelay && !safeModePending){
        Particle.process();
    }

    if(safeModePending){
        RGB.color(0,0,10);
        delay(1000);
        Particle.process();
        RGB.color(0,0,1);
        delay(1000);
        System.enterSafeMode();    
    }
    
    System.sleep(SLEEP_MODE_DEEP, INTERVAL);     // we get this far. DEEP SLEEP resets the device.
}

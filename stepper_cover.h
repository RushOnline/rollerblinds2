#ifndef __STEPPER_COVER_H__
#define __STEPPER_COVER_H__

#include "esphome.h"

#include <SoftwareSerial.h>
#include <TMCStepper.h>
#include <ESP8266TimerInterrupt.h>

#define	ENABLE_UART		0

#define MAX_STEPS		26100

// D1 GPIO5 - TMC DIR
// D2 GPIO4 - TMC STEP
// D4 GPIO2 (pu, HIGH at boot, LED, boot fails if pulled LOW) - TMC EN#

#define DRIVER_DIR			D1
#define DRIVER_STEP			D2
#define DRIVER_ENN			D4

// D6 GPIO12 -Top endstop (reed switch).
#define ENDSTOP_PIN			D6

#if (ENABLE_UART)
	#define DRIVER_UART_PIN	0		// TMC2209 SoftwareSerial TX/RX pin
	#define DRIVER_ADDRESS	0b00	// TMC2209 Driver address according to MS1 and MS2
	#define DRIVER_R_SENSE	0.11f	// Match to your driver
#endif//ENABLE_UART

#define HW_TIMER_FREQ	2000

long stepper_position = 0;
long stepper_target = 0;
long stepper_direction = 0;
long ignore_endstop = 0;

#define LOCK_TIMER		ITimer.disableTimer()
#define UNLOCK_TIMER	ITimer.enableTimer()

ESP8266Timer ITimer;
void IRAM_ATTR TimerHandler()
{
	static uint8_t step = 0;

	if (stepper_position == stepper_target) return;

	step = !step;

	if (step) {
		digitalWrite(DRIVER_STEP, HIGH);
		return;
	}

	digitalWrite(DRIVER_STEP, LOW);
	stepper_position += stepper_direction;

	if (ignore_endstop) --ignore_endstop;
}

bool at_endstop()
{
	return (digitalRead(ENDSTOP_PIN) == LOW);
}
	
class RushStepper
{
public:
	void enable() {
		digitalWrite(DRIVER_ENN, LOW);
	}

	void disable() {
		digitalWrite(DRIVER_ENN, HIGH);
	}

	void stop() {
		LOCK_TIMER;
		ESP_LOGD(__FILE__, "stepper.stop()");
		stepper_target = stepper_position;
		stepper_direction = 0;
		disable();
		UNLOCK_TIMER;
	}

	void set_position(long position) {
		LOCK_TIMER;
		stepper_position = position;
		stepper_direction = 0;
		UNLOCK_TIMER;
	}

	long get_position() {
		return stepper_position;
	}

	bool is_running() {
		return ((stepper_target != stepper_position) && stepper_direction);
	}

	void moveTo(long target) {
		LOCK_TIMER;
		stepper_target = target;
		stepper_direction = (stepper_target < stepper_position) ? -1 : 1;
		if (stepper_target == stepper_position) {
			stepper_direction = 0;
		} else {
			if (at_endstop()) ignore_endstop = 100;
			digitalWrite(DRIVER_DIR, (stepper_direction < 0) ? LOW : HIGH);
			enable();
		}
		UNLOCK_TIMER;
	}

	void setup() {
		pinMode(DRIVER_ENN, OUTPUT);
		pinMode(DRIVER_DIR, OUTPUT);
		pinMode(DRIVER_STEP, OUTPUT);

		digitalWrite(DRIVER_DIR, LOW);
		digitalWrite(DRIVER_STEP, LOW);

		ITimer.setFrequency(HW_TIMER_FREQ, TimerHandler);
		stop();
	}

	void run() {
		if (stepper_target != stepper_position) return;
		if (!stepper_direction) return;
		stepper_direction = 0;
		disable();
	}

protected:
	long position_{0};
};


class StepperCover : public PollingComponent, public Cover
{
public:

	StepperCover() : PollingComponent(1000) {}
	// float target{COVER_OPEN};

	CoverTraits get_traits() override
	{
		auto traits = CoverTraits();
		traits.set_supports_position(true);
		traits.set_supports_toggle(true);
		traits.set_is_assumed_state(false);
		traits.set_supports_tilt(false);
		return traits;
	}

	// This will be called by App.setup()
	void setup() override
	{
        ESP_LOGD(__FILE__, "* cover setup");

		stepper.setup();
		stepper.disable();
		stepper.stop();

#if (ENABLE_UART)
        ESP_LOGD(__FILE__, "UART setup");
		serial.enableIntTx(false);
		serial.begin(115200);

		driver.begin();
		driver.microsteps(2);
		driver.rms_current(1200); // mA
#endif//ENABLE_UART

		pinMode(ENDSTOP_PIN, INPUT_PULLUP);


		ESP_LOGD(__FILE__, "endstop: %s", (digitalRead(ENDSTOP_PIN) == LOW) ? "ON" : "OFF" );

		if (at_endstop())
		{
			position = COVER_OPEN;
		} else {
			position = COVER_CLOSED;
		}


        ESP_LOGD(__FILE__, "cover.position: %.2f, stepper.setCurrentPosition: %d", position, pos2steps(position));
		stepper.set_position(pos2steps(position));
		current_operation = COVER_OPERATION_IDLE;
		publish_state();

		// Interval in microsecs
		// if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler))
		// {
	    //     ESP_LOGD(__FILE__, "Interrupt timer set to %d us", HW_TIMER_INTERVAL_US);
		// } else {
	    //     ESP_LOGD(__FILE__, "Interrupt timer set failed");
		// }

	}

	// This will be called every time the user requests a state change.
	void control(const CoverCall &call) override
	{
        ESP_LOGD(__FILE__, "* cover control: cover: %.2f, stepper: %d", position, stepper.get_position());

		if (call.get_position().has_value())
		{
			ESP_LOGD(__FILE__, "cover.moveTo: %.1f, cover.position: %.1f", *call.get_position(), position);
			auto pos = *call.get_position();
			if (!settled && (pos == COVER_OPEN))
				settle();
			else
				moveTo(pos);
		}

		if (call.get_toggle().has_value() && *call.get_toggle())
		{
			// User requested cover toggle
			// if (last_operation == COVER_OPERATION_OPENING)
			// 	last_operation = COVER_OPERATION_CLOSING;
			// else
			// 	last_operation = COVER_OPERATION_OPENING;
		}

		if (call.get_stop())
		{
			// User requested cover stop
			stop();
		}
	}

	void loop() override
	{
		if (!ignore_endstop && at_endstop() && stepper.is_running()) {
			stepper.stop();
			ESP_LOGD(__FILE__, "endstop detected");
			position = COVER_OPEN;
			stepper.set_position(pos2steps(position));
			current_operation = COVER_OPERATION_IDLE;
			settled = true;
			publish_state();
		}

		stepper.run();
	}

	void settle() {
		ESP_LOGD(__FILE__, "* HOMING REQUESTED");
		settled = false;
		stepper.set_position(pos2steps(COVER_CLOSED*2));
		current_operation = COVER_OPERATION_CLOSING;
		last_operation = COVER_OPERATION_IDLE;
		position = COVER_CLOSED;
		stepper.moveTo(pos2steps(COVER_OPEN*2));
		publish_state();
	}

	void update() override {
		static CoverOperation op = COVER_OPERATION_IDLE;
		if (!stepper_direction && (op == COVER_OPERATION_IDLE)) return;

		position = steps2pos(stepper.get_position());
		if (stepper_direction < 0)
			current_operation = COVER_OPERATION_CLOSING;
		else if (stepper_direction > 0)
			current_operation = COVER_OPERATION_OPENING;
		else
			current_operation = COVER_OPERATION_IDLE;
		
		op = current_operation;
		publish_state();
	}

protected:

	CoverOperation last_operation{COVER_OPERATION_CLOSING};
	bool settled{false};

	float steps2pos(long steps)
	{
		return (steps / float(MAX_STEPS));
	}

	long pos2steps(float pos)
	{
		return pos * MAX_STEPS;
	}

	void moveTo(float new_position)
	{
		if (new_position == position) {
			stop();
			return;
		}
		last_operation = current_operation = (new_position > position) ? COVER_OPERATION_OPENING :  COVER_OPERATION_CLOSING;
		ESP_LOGD(__FILE__, "stepper.moveTo: %d, from: %d", pos2steps(new_position), stepper.get_position());
		stepper.moveTo(pos2steps(new_position));
		publish_state();
	}

	void stop()
	{
        ESP_LOGD(__FILE__, "* cover control: cover: %.2f, stepper: %d", position, stepper.get_position());
		stepper.stop();
		position = steps2pos(stepper.get_position());
		current_operation = COVER_OPERATION_IDLE;
		publish_state();
	}

//	AccelStepper stepper{AccelStepper::DRIVER, DRIVER_STEP, DRIVER_DIR};
	RushStepper stepper;

#if (ENABLE_UART)
	SoftwareSerial serial{DRIVER_UART_PIN};
	TMC2209Stepper driver{&serial, DRIVER_R_SENSE, DRIVER_ADDRESS};
#endif//ENABLE_UART

};

#endif //__STEPPER_COVER_H__

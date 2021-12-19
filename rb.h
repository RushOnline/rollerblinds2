#ifndef __RB__H__
#define __RB__H__

#include "esphome.h"

class StepperControl {
public:
    StepperControl() {
        ESP_LOGD("StepperControl", "CONSTRUCTOR");

        max_speed_ = 0;
        max_speed_ = 0;
        xceleration_ = 0;
    }

    /**
     * Setup stepper
     */
    void config(int32_t max_steps, int32_t max_speed, float xceleration)
    {
        ESP_LOGD("StepperControl", "before: max_steps: %d, max_speed: %d, xceleration: %0.2f", max_steps_, max_speed_, xceleration_);
        ESP_LOGD("StepperControl", "after:  max_steps: %d, max_speed: %d, xceleration: %0.2f", max_steps, max_speed, xceleration);

        max_steps_ = max_steps;
        max_speed_ = max_speed;
        xceleration_ = xceleration;

        xstepper->set_acceleration(xceleration);
        xstepper->set_deceleration(xceleration);
        xstepper->set_max_speed(max_speed);
    }

    /**
     * Stop stepper slow when curtain is about to close.
     * 
     * This function is required because the curtain is too heavy to the 4:1 gearbox,
     * so we need to stop rolling very slow at the bottom and can stop within a moment at a top
     */
    void stop() {
        // d0 = v * v / (2 * a)
        // far from top -> allow more distance to stop
        // d1 = d0 * (cp / tp) = v * v / (2 * a) * (cp / tp)

        ESP_LOGD("StepperControl", "stop: max_steps: %d, max_speed: %d, xceleration: %0.2f", max_steps_, max_speed_, xceleration_);

        int brakepath = int(max_speed_ * max_speed_ / (2 * xceleration_) * xstepper->current_position / max_steps_);

        // how many upside/downside steps left to target?
        int diff = xstepper->target_position - xstepper->current_position;

        if (diff < 0)
        {
            if (-diff > brakepath)
            {
                xstepper->set_target(xstepper->current_position - brakepath);
            }
        }

        if (diff > 0)
        {
            if (diff > brakepath)
            {
                xstepper->set_target(xstepper->current_position + brakepath);
            }
        }
    }

protected:
    int32_t max_steps_;
    int32_t max_speed_;
    float xceleration_;
};

StepperControl stepper_control;

#endif//__RB__H__

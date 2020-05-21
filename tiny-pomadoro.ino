#include "Arduino.h"

const int PIN_BUZZER    = 0;
const int PIN_GREEN_LED = 1;
const int PIN_BUTTON    = 2;
const int PIN_RED_LED   = 4;

const int NUM_OUTPUTS   = 3;

const long WORK_DURATION  = 1500000;
const long BREAK_DURATION = 300000;

enum State{
    PENDING_START,
    WORK,
    BREAK,
    SLEEPING,
};

struct OutputState{
    int pin;
    bool on;
    long currentStateDuration;
};

State state = PENDING_START;
long stateDuration = 0;

long deltaMillis = 0;

long buttonDownLoops = 0;
bool buttonPressed = false;
long buttonPressDuration = 0;

long buzzerBeepStart = 0;
bool buzzerBeeping = false;

OutputState buzzerState = {.pin = PIN_BUZZER, .on = false, .currentStateDuration = 0};
OutputState redLEDState = {.pin = PIN_RED_LED, .on = false, .currentStateDuration = 0};
OutputState greenLEDState = {.pin = PIN_GREEN_LED, .on = false, .currentStateDuration = 0};

OutputState *outputs[NUM_OUTPUTS] = {&buzzerState, &redLEDState, &greenLEDState};

void setup(){
    pinMode(PIN_RED_LED, OUTPUT);
    pinMode(PIN_GREEN_LED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_BUTTON, INPUT);

    setOutputState(&greenLEDState, true);
}

void loop(){
    long startMillis = millis();
    stateDuration += deltaMillis;
    updateButton();
    updateOutputs();
    updateBuzzer();
    
    if(buttonPressDuration >= 1000 && stateDuration >= 1000){
        if(state == SLEEPING){
            changeState(WORK);
        }else{
            changeState(SLEEPING);
        }
    }

    switch (state){
        case PENDING_START:
            updatePendingStart();
            break;
        case WORK:
            updateWork();
            break;
        case BREAK:
            updateBreak();
            break;
        case SLEEPING:
            updateSleeping();
            break;
    }

    deltaMillis = millis() - startMillis;
}

void updatePendingStart(){
    if (greenLEDState.on && greenLEDState.currentStateDuration > 500){
        setOutputState(&greenLEDState, false);
        setOutputState(&redLEDState, true);
    }
    else if (redLEDState.currentStateDuration > 500){
        setOutputState(&greenLEDState, true);
        setOutputState(&redLEDState, false);
    }

    if (buttonPressed){
        changeState(WORK);
    }
}

void updateWork(){
    setOutputState(&greenLEDState, true);
    setOutputState(&redLEDState, false);
    if(stateDuration > WORK_DURATION){
        changeState(BREAK);
    }
}

void updateBreak(){
    setOutputState(&greenLEDState, false);
    setOutputState(&redLEDState, true);
    if(stateDuration > WORK_DURATION){
        changeState(WORK);
    }
}

void updateSleeping(){
    setOutputState(&greenLEDState, false);
    setOutputState(&redLEDState, false);
}

void updateOutputs(){
    for (int i = 0; i < NUM_OUTPUTS; i++){
        outputs[i]->currentStateDuration += deltaMillis;
        digitalWrite(outputs[i]->pin, outputs[i]->on);
    }
}

void setOutputState(OutputState *state, bool on){
    state->on = on;
    state->currentStateDuration = 0;
}

void updateButton(){
    bool buttonDown = digitalRead(PIN_BUTTON);
    if (buttonDown){
        buttonDownLoops += 1;
    }else {
        buttonDownLoops = 0;
    }

    buttonPressed = buttonDownLoops >= 5;
    
    if(buttonPressed){
        buttonPressDuration += deltaMillis;
    }else{
        buttonPressDuration = 0;
    }
}

void updateBuzzer(){
    if (buzzerBeeping && millis() - buzzerBeepStart > 500){
        buzzerBeeping = false;
    }
    digitalWrite(PIN_BUZZER, buzzerBeeping);
}

void beep(){
    buzzerBeepStart = millis();
    buzzerBeeping = true;
}

void changeState(State newState){
    beep();
    state = newState;
    stateDuration = 0;
}
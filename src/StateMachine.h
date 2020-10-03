#include <LinkedList.h>
#include "State.h"

#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

//#define DEBUG_FSM
#define DEBUGFSM(fmt, ...)
#ifdef DEBUG_FSM
#ifdef DEBUG_ESP_PORT
#define DEBUGFSM(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ##__VA_ARGS__ )
#endif
#endif

class StateMachine
{
  public:
    // Methods
    
    StateMachine();
    StateMachine(unsigned long intervall);
    StateMachine(Print& debug);
    StateMachine(unsigned long intervall, Print& debug);
    ~StateMachine();
    void init();
    void run();
    void update();

    State* addState(const char* name, void (*functionPointer)());
    State* addState(const char* name);
    State* transitionTo(State* s);
    int transitionTo(int i);
    State* getCurrentState();
	
    // Attributes
    //Print& debug;
    LinkedList<State*> *stateList;
	  bool executeOnce = true; 	//Indicates that a transition to a different state has occurred
    int currentState = -1;	//Indicates the current state number
    int previousState = -1;
    unsigned long intervall = 0;
    unsigned long latestRun = 0;
    unsigned long latestStateChange = 0;
};

StateMachine::StateMachine(){
  this->stateList = new LinkedList<State*>();
  this->intervall = 0;
};

StateMachine::StateMachine(unsigned long intervall = 0){
  this->stateList = new LinkedList<State*>();
  this->intervall = intervall;
};
/*
StateMachine::StateMachine(Print& debug){
  this->stateList = new LinkedList<State*>();
  this->intervall = 0;
};

StateMachine::StateMachine(unsigned long intervall = 0, Print& debug){
  this->stateList = new LinkedList<State*>();
  this->intervall = intervall;
};
*/
StateMachine::~StateMachine(){};

/*
 * Adds a state to the machine
 * It adds the state in sequential order.
 * 
 * DEPRECATED. Use addState(name) and onUpdate() instead
 */
State* StateMachine::addState(const char* name, void(*functionPointer)()){
  State* s = new State(name);
  s->updateLogic = functionPointer;
  stateList->add(s);
  s->index = stateList->size()-1;
  return s;
}

/*
 * Adds a state to the machine
 * It adds the state in sequential order.
 */
State* StateMachine::addState(const char* name){
  State* s = new State(name);
  stateList->add(s);
  s->index = stateList->size()-1;
  return s;
}

/*
 * Jump to a state
 * given by a pointer to that state.
 */
State* StateMachine::transitionTo(State* s){
  this->currentState = s->index;
  this->executeOnce = true;
  return s;
}

/*
 * Jump to a state
 * given by a state index number.
 */
int StateMachine::transitionTo(int i){
  if(i < stateList->size()){
  	this->currentState = i;
	  this->executeOnce = true;
	  return i;
  }
  return currentState;
}

State* StateMachine::getCurrentState(){
  if (currentState == -1) return NULL;
  else return stateList->get(currentState);
}

/*
 * Initialize the state machine by "transitioning" to the initial state (state 0).
 * This shoud be called after setting up all states and transitions.
*/
void StateMachine::init() {
  if(stateList->size() == 0) return;

  if(currentState == -1) {
    executeOnce = true;
    latestStateChange = millis();
    currentState = 0;
    previousState = 0;
    DEBUG_ESP_PORT.printf("---\nInitial state: %s\n", getCurrentState()->name);
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->entryLogic) getCurrentState()->entryLogic();
    return;
  }
}

void StateMachine::update(){
  if(currentState == -1 || (unsigned long)(millis() - latestRun) >= intervall){
    latestRun = millis();
    run();
  }
} 

/*
 * Main execution of the machine occurs here in run
 * The current state is executed and it's transitions are evaluated
 * to determine the next state. 
 * 
 * If init() wasn't explicitly called before, it gets called at the first execution of run().
 * 
 * By design, only one state is executed in one loop() cycle.
 */
void StateMachine::run(){

  if (stateList->size() == 0) return;
  if (currentState == -1) init();
  
  //
  // call the updte callback on the current state
  //

  DEBUGFSM("--- Current state: %s\n", getCurrentState()->name);
  if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();

  //
  // find out the next state
  //

  int nextState;

  // check if we ran into the timeout condition and continue with the timeout state
  if (getCurrentState()->timeout > 0 && (unsigned long)(millis() - latestStateChange) > getCurrentState()->timeout) {
    DEBUGFSM("Timeout of state: %s\n", getCurrentState()->name);
    nextState = getCurrentState()->evalTimeoutTransition();
  }
  // check if the state is debouncew-locked and stay  
  else if ((unsigned long)(millis() - latestStateChange) < getCurrentState()->debounce) {
    DEBUGFSM("Locked (debouncing) in  state: %s\n", getCurrentState()->name);
    nextState = currentState;
  }
  // otherwise evaluate the stats transitions
  else {
    nextState = getCurrentState()->evalTransitions();
  }

  //
  // Advance to the next state and execute the callbacks
  //

  if (nextState != currentState) {
    DEBUGFSM("Transition to: %s\n", stateList->get(nextState)->name);
    if (getCurrentState()->exitLogic) getCurrentState()->exitLogic();

    executeOnce = true;
    latestStateChange = millis();
    previousState = currentState;
    currentState = nextState;

    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->entryLogic) getCurrentState()->entryLogic();
  }
  else {
      executeOnce = false;
  }
}

#endif

#include <LinkedList.h>
#include "State.h"

#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

class StateMachine
{
  public:
    // Methods
    
    StateMachine();
    StateMachine(unsigned long intervall);
    ~StateMachine();
//    void init();
    void run();
    void update();

    State* addState(const char* name, void (*functionPointer)());
    State* addState(const char* name);
    State* transitionTo(State* s);
    int transitionTo(int i);
    State* getCurrentState();
	
    // Attributes
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
  return stateList->get(currentState);
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
 * By design, only one state is executed in one loop() cycle.
 */
void StateMachine::run(){
  // Early exit, no states are defined
  if(stateList->size() == 0) return;

  //
  // if this is the very first run, "transition" to the initial state (state 0)
  //

  if(currentState == -1) {
    executeOnce = true;
    latestStateChange = millis();
    currentState = 0;
    previousState = 0;
    Serial.printf("---\nInitial state: %s\n", getCurrentState()->name);
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->entryLogic) getCurrentState()->entryLogic();
    return;
  }
  
  //
  // call the updte callback on the current state
  //

  Serial.printf("---\nCurrent state: %s\n", getCurrentState()->name);
  if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();

  //
  // find out the next state
  //

  int nextState;

  // check if we ran into the timeout condition and continue with the timeout state
  if (getCurrentState()->timeout > 0 && (unsigned long)(millis() - latestStateChange) > getCurrentState()->timeout) {
    Serial.printf("Timeout of state: %s\n", getCurrentState()->name);
    nextState = getCurrentState()->evalTimeoutTransition();
  }
  // check if the state is debouncew-locked and stay  
  else if ((unsigned long)(millis() - latestStateChange) < getCurrentState()->debounce) {
    Serial.printf("Locked (debouncing) in  state: %s\n", getCurrentState()->name);
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
    Serial.printf("Transition to: %s\n", stateList->get(nextState)->name);
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

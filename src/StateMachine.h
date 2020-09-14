#include <LinkedList.h>
#include "State.h"

#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

class StateMachine
{
  public:
    // Methods
    
    StateMachine();
    ~StateMachine();
    void init();
    void run();

    // When a stated is added we pass the function that represents 
    // that state logic
    State* addState(const char* name, void (*functionPointer)());
    State* transitionTo(State* s);
    int transitionTo(int i);
    State* getCurrentState();
	
    // Attributes
    LinkedList<State*> *stateList;
	  bool executeOnce = true; 	//Indicates that a transition to a different state has occurred
    int currentState = -1;	//Indicates the current state number
    int previousState = -1;
    unsigned long latestStateChange = 0;
};

StateMachine::StateMachine(){
  stateList = new LinkedList<State*>();
};

StateMachine::~StateMachine(){};

/*
 * Main execution of the machine occurs here in run
 * The current state is executed and it's transitions are evaluated
 * to determine the next state. 
 * 
 * By design, only one state is executed in one loop() cycle.
 */
void StateMachine::run(){
  //Serial.println("StateMachine::run()");
  // Early exit, no states are defined
  if(stateList->size() == 0) return;

  // Initial condition
  if(currentState == -1){
    currentState = 0;
    latestStateChange = millis();
  }
  
  int nextState = currentState;

  Serial.printf("---\nCurrent state: %s\n", getCurrentState()->name);

  // Execute state logic and return transitioned
  // to state number. 

  if (getCurrentState()->timeout > 0 && (unsigned long)(millis() - latestStateChange) > getCurrentState()->timeout) {
    Serial.printf("Timeout!\n");
    nextState = getCurrentState()->evalTimeoutTransition();
  }
  else {
    nextState = getCurrentState()->execute();
  }

  if (nextState != currentState) {
    Serial.printf("Transition to: %s\n", stateList->get(nextState)->name);
    executeOnce = true;
    latestStateChange = millis();
    previousState = currentState;
    currentState = nextState;
  }
  else {
    executeOnce = false;
  }
}

/*
 * Adds a state to the machine
 * It adds the state in sequential order.
 */
State* StateMachine::addState(const char* name, void(*functionPointer)()){
  State* s = new State(name);
  s->stateLogic = functionPointer;
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

#endif

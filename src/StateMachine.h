#include <LinkedList.h>
#include "State.h"

#ifndef _STATEMACHINE_H
#define _STATEMACHINE_H

class StateMachine
{
  public:
    // Methods
    
    StateMachine(unsigned long intervall);
    ~StateMachine();
    void init();
    void run();
    void update();

    // When a stated is added we pass the function that represents 
    // that state logic
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

StateMachine::StateMachine(unsigned long intervall = 0){
  this->stateList = new LinkedList<State*>();
  this->intervall = intervall;
};

StateMachine::~StateMachine(){};

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
  //Serial.println("StateMachine::run()");
  // Early exit, no states are defined
  if(stateList->size() == 0) return;

  int nextState;
  executeOnce = false;

  // Initial condition
  if(currentState == -1){
    executeOnce = true;
    latestStateChange = millis();
    currentState = 0;
    previousState = 0;
    Serial.printf("---\nInitial state: %s\n", getCurrentState()->name);
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->entryLogic) getCurrentState()->entryLogic();
    return;
  }
  
  Serial.printf("---\nCurrent state: %s\n", getCurrentState()->name);
 
  if (getCurrentState()->timeout > 0 && (unsigned long)(millis() - latestStateChange) > getCurrentState()->timeout) {
    Serial.printf("Timeout of state: %s\n", getCurrentState()->name);
    nextState = getCurrentState()->evalTimeoutTransition();
  }
  else if ((unsigned long)(millis() - latestStateChange) >= getCurrentState()->debounce) {
    nextState = getCurrentState()->evalTransitions();
  }
  else {
    Serial.printf("Locked (debouncing) in  state: %s\n", getCurrentState()->name);
    nextState = currentState;
  }

  if (nextState != currentState) {
    executeOnce = true;
    latestStateChange = millis();
    previousState = currentState;
    Serial.printf("Transition to: %s\n", stateList->get(nextState)->name);
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->exitLogic) getCurrentState()->exitLogic();
    currentState = nextState;
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
    if (getCurrentState()->entryLogic) getCurrentState()->entryLogic();
  }
  else {
    if (getCurrentState()->updateLogic) getCurrentState()->updateLogic();
  }
//  else {
//    executeOnce = false;
//  }
}

/*
 * Adds a state to the machine
 * It adds the state in sequential order.
 */
State* StateMachine::addState(const char* name, void(*functionPointer)()){
  State* s = new State(name);
  s->updateLogic = functionPointer;
  stateList->add(s);
  s->index = stateList->size()-1;
  return s;
}

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

#endif

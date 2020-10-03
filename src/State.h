#include <LinkedList.h>

#ifndef _STATE_H
#define _STATE_H

// GUH

/*
 * Transition is a structure that holds the address of 
 * a function that evaluates whether or not not transition
 * from the current state and the number of the state to transition to
 */
struct Transition{
  bool (*conditionFunction)();
  int stateNumber;
};

/*
 * State represents a state in the statemachine. 
 * It consists mainly of the address of the function
 * that contains the state logic and a collection of transitions 
 * to other states.
 */
class State{
  public:
    State(const char* name);
    ~State();

	  State* onUpdate(void (*functionPointer)());
	  State* onEntry(void (*functionPointer)());
	  State* onExit(void (*functionPointer)());

	  State* addTransition(bool (*c)(), State* s);
    State* addTransition(bool (*c)(), int stateNumber);
	  
    State* setTimeout(unsigned long timeout_ms, State* s);
	  State* setTimeout(unsigned long timeout_ms, int stateNumber);
	  State* setTimeout(unsigned long timeout_ms, bool (*c)(), State* s);
    State* setTimeout(unsigned long timeout_ms, bool (*conditionFunction)(), int stateNumber);
    
    State* setDebounce(unsigned long debounce_ms);
    
    int evalTransitions();
    int evalTimeoutTransition();
    int execute();
    int setTransition(int index, int stateNumber);	//Can now dynamically set the transition

    void (*updateLogic)();
    void (*entryLogic)();
    void (*exitLogic)();

	  int index;
    const char* name;
    LinkedList<struct Transition*> *transitions;
    Transition *timeoutTransition; 
    unsigned long timeout = 0;
    unsigned long debounce = 0;
};

State::State(const char* name){
  transitions = new LinkedList<struct Transition*>();
  this->name = name;
};

State::~State(){};

State* State::onUpdate(void (*functionPointer)()){
  updateLogic = functionPointer;
  return this;
}

State* State::onEntry(void (*functionPointer)()){
  entryLogic = functionPointer;
  return this;
}

State* State::onExit(void (*functionPointer)()){
  exitLogic = functionPointer;
  return this;
}

/*
 * Adds a transition structure to the list of transitions
 * for this state.
 * Params:
 * conditionFunction is the address of a function that will be evaluated
 * to determine if the transition occurs
 * state is the state to transition to
 */
State* State::addTransition(bool (*conditionFunction)(), State* s){
  return addTransition(conditionFunction, s->index);
}

/*
 * Adds a transition structure to the list of transitions
 * for this state.
 * Params:
 * conditionFunction is the address of a function that will be evaluated
 * to determine if the transition occurs
 * stateNumber is the number of the state to transition to
 */
State* State::addTransition(bool (*conditionFunction)(), int stateNumber){
  struct Transition* t = new Transition{conditionFunction, stateNumber};
  transitions->add(t);
  return this;
}


State* State::setTimeout(unsigned long timeout_ms, State* s){
  return setTimeout(timeout_ms, s->index);
}

State* State::setTimeout(unsigned long timeout_ms,  int stateNumber){
  return setTimeout(timeout_ms, [](){return true;}, stateNumber);
}

State* State::setTimeout(unsigned long timeout_ms, bool (*conditionFunction)(), State* s){
  return setTimeout(timeout_ms, conditionFunction, s->index);
}

State* State::setTimeout(unsigned long timeout_ms, bool (*conditionFunction)(), int stateNumber){
  struct Transition* t = new Transition{conditionFunction, stateNumber};
  timeoutTransition = t;
  timeout = timeout_ms;
  return this;
}

State* State::setDebounce(unsigned long debounce_ms){
  debounce = debounce_ms;
  return this;
}

/*
 * Evals all transitions sequentially until one of them is true.
 * Returns:
 * The stateNumber of the transition that evaluates to true
 * -1 if none evaluate to true ===> Returning index now instead to avoid confusion between first run and no transitions
 */
int State::evalTransitions(){
  for(int i=0; i<transitions->size(); i++) {
    if(transitions->get(i)->conditionFunction()){
      return transitions->get(i)->stateNumber;
    }
  }
  return index;
}

int State::evalTimeoutTransition(){
  if (timeoutTransition && timeoutTransition->conditionFunction()) {
    return timeoutTransition->stateNumber;
  }
  return index;
}

/*
 * Method to dynamically set a transition
 */
int State::setTransition(int index, int stateNo){
	if(transitions->size() == 0) return -1;
	transitions->get(index)->stateNumber = stateNo;
	return stateNo;
}


#endif

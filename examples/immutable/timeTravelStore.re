open SimpleStore;

type stringAction =
  | A
  | B;

let stringReduce = (state, action) =>
  switch action {
  | A => state ++ "a"
  | B => state ++ "b"
  };

type ReduxThunk.thunk(_) +=
  | StringAction (stringAction)
  | CounterAction (action);

type ReduxThunk.thunk('a) +=
  | ReplaceState ('a);

let appReducter = (state: AppState.appState, action) =>
  switch action {
  | StringAction(action) => {...state, notACounter: stringReduce(state.notACounter, action)}
  | CounterAction(action) => {...state, counter: counter(state.counter, action)}
  | ReplaceState(replacedState) => replacedState
  | _ => state
  };

type ReduxThunk.thunk(_) +=
  | TravelBackward
  | TravelForward;

let past = ref(Immutable.Stack.empty());

let future = ref(Immutable.Stack.empty());

let goBack = (currentState) =>
  switch (Immutable.Stack.first(past^)) {
  | Some(lastState) =>
    future := Immutable.Stack.addFirst(currentState, future^);
    if (Immutable.Stack.isNotEmpty(past^)) {
      past := Immutable.Stack.removeFirstOrRaise(past^)
    };
    lastState
  | None => currentState
  };

let goForward = (currentState) =>
  switch (Immutable.Stack.first(future^)) {
  | Some(nextState) =>
    past := Immutable.Stack.addFirst(currentState, past^);
    if (Immutable.Stack.isNotEmpty(future^)) {
      future := Immutable.Stack.removeFirstOrRaise(future^)
    };
    nextState
  | None => currentState
  };

let recordHistory = (currentState) => {
  past := Immutable.Stack.addFirst(currentState, past^);
  future := Immutable.Stack.empty()
};

let timeTravel = (store, next, action) => {
  let currentState = Reductive.Store.getState(store);
  switch action {
  | TravelBackward => next(ReplaceState(goBack(currentState)))
  | TravelForward => next(ReplaceState(goForward(currentState)))
  | _ =>
    next(action);
    let newState = Reductive.Store.getState(store);
    if (currentState !== newState) {
      recordHistory(currentState)
    }
  }
};

let thunkedLoggedTimeTravelLogger = (store, next) =>
  Middleware.thunk(store) @@ Middleware.logger(store) @@ timeTravel(store) @@ next;

let store =
  Reductive.Store.create(
    ~reducer=appReducter,
    ~preloadedState={counter: 0, notACounter: ""},
    ~enhancer=thunkedLoggedTimeTravelLogger,
    ()
  );

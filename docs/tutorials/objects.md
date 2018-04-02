Introduction to objects
=======================

In SurgeScript, objects are units of code and data. Each object define a finite state machine, a mathematical construct used to describe the behavior of in-game objects. But what's a finite state machine?

State machines
--------------

A finite state machine is a mathematical abstraction composed of a finite number of **states** and by **transitions** between these states. Only one state can be active at any given time.

As a concrete example, suppose that you want to create a door in your game. You, the developer, may open or close the door at will.

That is said to be an *informal* description of the problem. We need to transform it in something more rigorous. What is a door? We'll show one possible solution here.

A door is an object with two states (see Figure below):

- Opened
- Closed

![State machine](../img/state-machine.png)

From the Opened state, only one transition is possible: Close. Once the Close transition is called, the door will go to the Closed state. Conversely, from the Closed state, only one transition is available: Open. When the Open transition is executed, the door will return to the Opened state.

Objects in SurgeScript
----------------------

In SurgeScript, each object has a name. Objects may have any number of states you'd like. The initial active state is always called **main**. The syntax is as follows:

```
object "name of the object"
{
    state "state name"
    {
        // code goes here
    }
}
```

The following code shows an example of a door that opens and closes (it keep changing states) every 2 seconds:

```
object "Cosmic Door"
{
    state "main"
    {
        // the object starts at the main state
        state = "opened"; // go to the opened state
    }

    state "opened"
    {
        if(timeout(2)) // if we have been on the opened state for 2 seconds
            state = "closed"; // go to the closed state
    }

    state "closed"
    {
        if(timeout(2))
            state = "opened";
    }
}
```

Notice how the code just shown defines states and transitions between these states.

**Note:** once a state is active, its code will be repeated (at every frame) until the state changes or until the object is destroyed.
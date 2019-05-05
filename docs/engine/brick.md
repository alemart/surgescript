Brick
=====

The Brick component is used to create brick-like objects, or bricks endowed with scripting capabilities. This allows you to create elevators, conditional barriers (e.g., doors), movable platforms with custom trajectories and much more.

The Brick component only affects collisions. The object that uses it is required to be an [entity](entity). **This component doesn't do any rendering.** Therefore, it's typically used in conjunction with the [Actor](actor) component.

*Example*
```
using SurgeEngine.Actor;
using SurgeEngine.Brick;

object "On/Off Platform" is "entity"
{
    actor = Actor("On/Off Platform"); // for rendering
    brick = Brick("On/Off Platform"); // for collision

    state "main"
    {
        if(timeout(2.0)) {
            brick.enabled = false;
            state = "disabled";
        }
    }

    state "disabled"
    {
        if(timeout(2.0)) {
            brick.enabled = true;
            state = "main";
        }
    }
}
```



Factory
-------

#### Brick

`Brick(spriteName)`

Spawns a new Brick component. Just like regular bricks, a Brick component has a collision mask associated with it. By default, the collision mask will be computed according to the animation 0 of the sprite named `spriteName`. 

*Arguments*

* `spriteName`: string. The name of a sprite.

*Returns*

A new Brick component.


Properties
----------

#### enabled

`enabled`: boolean.

Whether the Brick component should affect collisions or not. Defaults to `true`.

#### type

`type`: string.

The type of the Brick component specifies how it affects collisions. This value must be either *"solid"* or *"cloud"*. Defaults to *"solid"*.

#### layer

`layer`: string.

Should the Brick component be tied to a specific layer? If so, which layer? This value must be one of the following: *"green"*, *"yellow"*, *"default"*. Defaults to *"default"*.
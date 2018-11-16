Sound
=====

The Sound object lets you play samples, which are short sounds like: jump, brake, select, hit, etc. Sounds are loaded entirely in the memory; therefore, this object is meant to be used only with samples. If you need to play longer things like music, consider using [Music](music) instead.

*Example*
```
using SurgeEngine.Audio.Sound;

// will play a sound every 5 seconds
object "SoundTest"
{
    sound = Sound("samples/jump.wav");

    state "main"
    {
        sound.play();
        state = "wait";
    }

    state "wait"
    {
        if(timeout(5.0))
            state = "main";
    }
}
```

Factory
-------

#### Sound

`SurgeEngine.Audio.Sound(path)`

Creates a Sound object associated with a certain file.

*Arguments*

* `path`: string. The path of the sound - usually a file in the *samples/* folder.

*Returns*

A Sound object.

Properties
----------

#### playing

`playing`: boolean, read-only.

Will be `true` if the sound is playing.

#### volume

`volume`: number.

The volume of the sound, a value between 0.0 and 1.0, inclusive (zero means silence).

Functions
---------

#### play

`play()`

Plays the sound.

#### stop

`stop()`

Stops the sound.

Music
=====

The Music object lets you play music. This is meant to handle background music; therefore, only one music can be played at a time.

Unlike regular sounds, musics are streamed, (i.e., not loaded completely in the memory). Given the longer nature of musics, this improves memory usage and helps with the loading times.

*Example*
```
using SurgeEngine.Level;

// Fades the music in a linear fashion ;)
object "MyMusicFader"
{
    public music = Level.music;
    prevVolume = music.volume;
    fadeTime = 1.0; // given in seconds

    state "main"
    {
    }

    state "fadeout"
    {
        music.volume -= Time.delta / fadeTime;
        if(music.volume <= 0.0) {
            music.pause();
            music.volume = prevVolume;
            state = "main";
        }
    }

    // call fadeOut() to make the
    // music fade smoothly
    fun fadeOut(seconds)
    {
        if(state == "main")
            prevVolume = music.volume;
        fadeTime = seconds;
        state = "fadeout";
    }
}
```

Factory
-------

#### Music

`SurgeEngine.Audio.Music(path)`

Creates a Music object associated with a certain file.

*Arguments*

* `path`: string. The path of the music - usually a file in the *musics/* folder.

*Returns*

A Music object.

*Example*
```
using SurgeEngine.Audio.Music;

object "MusicTest"
{
    music = Music("musics/options.ogg");

    state "main"
    {
        // loop music
        if(!music.playing)
            music.play();
    }
}
```

Properties
----------

#### playing

`playing`: boolean, read-only.

Will be `true` if the music is playing.

#### volume

`volume`: number.

The volume of the music, a value between 0.0 and 1.0, inclusive (zero means silence).

Functions
---------

#### play

`play()`

Plays the music (once). To make it loop, play it whenever `music.playing` is `false`.

#### stop

`stop()`

Stops the music. Once the music is stopped, it can only be played again from the beginning.

#### pause

`pause()`

Pauses the music. A music that is paused can be resumed later.

#### resume

`resume()`

Resumes a previously paused music.

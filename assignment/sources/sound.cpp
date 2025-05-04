#include "../includes/sound.h"

#ifdef __MINGW32__
#include <windows.h>
#else
#include <stdio.h>
// #include <stdlib.h>

#include <canberra.h>
#endif

#ifndef __MINGW32__
static ca_context* atgSoundContext = NULL;
#endif

void ATG::Sound::initialize() {
#ifndef __MINGW32__
    ca_context_create(&atgSoundContext);

    ca_context_change_props(
        atgSoundContext,
        CA_PROP_APPLICATION_ID,
        "io.github.RunicSpear.TankGame",
        NULL
    );

    ca_context_change_props(
        atgSoundContext,
        CA_PROP_CANBERRA_CACHE_CONTROL,
        "volatile",
        NULL
    );

    ca_context_change_props(
        atgSoundContext,
        CA_PROP_APPLICATION_NAME,
        "Andreas Tank Game",
        NULL
    );
#endif
}

void ATG::Sound::playSoundOnce(const char* filename) {
#ifdef __MINGW32__
    PlaySoundA(filename, NULL, SND_ASYNC|SND_FILENAME);
#else
    int err = ca_context_play(atgSoundContext, 0, CA_PROP_MEDIA_FILENAME, filename, NULL);

    if (err != 0) {
        printf("Playing sound failed: %s\n", ca_strerror(err));
    }
#endif
}

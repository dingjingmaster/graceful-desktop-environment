/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software.
 */

#include "session-launcher.h"

#ifndef GRACEFUL_SESSION_BINDIR
#define GRACEFUL_SESSION_BINDIR "/usr/bin"
#endif

GStrv graceful_session_launcher_build_mutter_argv (const char* sessionPath)
{
    const char* effectiveSessionPath = sessionPath;

    if (effectiveSessionPath == NULL || effectiveSessionPath[0] == '\0') {
        effectiveSessionPath = GRACEFUL_SESSION_BINDIR "/graceful-session";
    }

    return g_strdupv (
        (GStrv) (char*[]) {
            "mutter",
            "--wayland",
            "--display-server",
            "--",
            (char*) effectiveSessionPath,
            NULL
        }
    );
}

/*
 * Copyright (c) 2026 dingjing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software.
 */
#ifndef GRACEFUL_SESSION_LAUNCHER_H
#define GRACEFUL_SESSION_LAUNCHER_H

#include <glib.h>

G_BEGIN_DECLS

GStrv graceful_session_launcher_build_mutter_argv (const char* sessionPath);

G_END_DECLS

#endif

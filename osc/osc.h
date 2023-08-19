#ifndef OSC_H
#define OSC_H

#define TMUX_OSC_PREFIX "\033Ptmux;\033\033] putty-nd;"
#define TMUX_OSC_PREFIX_LEN (sizeof(TMUX_OSC_PREFIX) - 1)
#define TMUX_OSC_SUFFIX "\a\033\\"
#define TMUX_OSC_SUFFIX_LEN (sizeof(TMUX_OSC_SUFFIX) - 1)

#define OSC_PREFIX "\033] putty-nd;"
#define OSC_PREFIX_LEN (sizeof(OSC_PREFIX) - 1)
#define OSC_SUFFIX "\a"
#define OSC_SUFFIX_LEN (sizeof(OSC_SUFFIX) - 1)

#define OSC_MAX_RAW_CONTENT_SIZE 4096
#define OSC_MAX_ENCODED_PKG_SIZE 8192

#define OSC_VERSION 1

#endif /* OSC_H */


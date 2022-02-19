# ----------------------------
# Makefile Options
# ----------------------------

NAME = FLAPPY
ICON = icon.png
DESCRIPTION = "Flappy Bird"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

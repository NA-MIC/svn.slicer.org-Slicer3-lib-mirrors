#
# $Id: xpTheme.tcl,v 1.15 2004/03/22 16:19:38 jenglish Exp $
#
# Tile widget set: XP Native theme
#
# @@@ todo: spacing and padding needs tweaking

namespace eval tile {

style theme settings xpnative {

    # NOTE: MS documentation says to use "Tahoma 8" in Windows 2000 / XP,
    # although many MS programs still use "MS Sans Serif 8"
    #
    style default . -font "Tahoma 8" -foreground SystemWindowText
    style map "." -foreground [list disabled SystemGrayText]

    style default TButton -padding "15 0"
    style default TMenubutton -padding {8 4}

    style default Tab.TNotebook -padding {10 2 10 2}
    style map Tab.TNotebook -padding { selected {10 2 10 5} }

    style default TLabelframe -foreground #0046d5

    style default Toolbutton -padding {8 4}
} }

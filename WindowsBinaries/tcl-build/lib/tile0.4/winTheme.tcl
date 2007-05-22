#
# $Id: winTheme.tcl,v 1.14 2004/04/23 00:49:39 hobbs Exp $
#
# Tile widget set: Windows Native theme
#

namespace eval tile {

style theme settings winnative {

    variable WinGUIFont "{MS Sans Serif} 8"
    if {$tcl_platform(platform) eq "windows"
	&& $tcl_platform(osVersion) >= 5.0} {
	set WinGUIFont "Tahoma 8"
    }

    style default "." \
    	-background SystemButtonFace \
	-troughcolor SystemScrollbar \
	-foreground SystemWindowText \
	-font $WinGUIFont \
	;

    style map "." -foreground [list disabled SystemGrayText]

    style default TButton -width -11 -relief raised -shiftrelief 1
    style default TCheckbutton -padding "0 2"
    style default TRadiobutton -padding "0 2"
    style default TMenubutton -padding "8 4" -arrowsize 3 -relief raised

    style map TButton -relief {{!disabled pressed} sunken}

    style default Toolbutton -relief flat -padding {8 4}
    style map Toolbutton \
    	-relief {disabled flat selected sunken  pressed sunken  active raised}

    style default TScrollbar -troughrelief flat -borderwidth 0
    style default TScale -groovewidth 4 -sliderrelief raised
    style map TScale -sliderrelief {pressed sunken}

    style default Tab.TNotebook -padding {12 3 12 3}
    style map Tab.TNotebook -padding [list selected {12 3 12 5}]

    style default TProgress \
        -borderwidth 1 \
        -background SystemHighlight \
        -sliderrelief sunken

} }

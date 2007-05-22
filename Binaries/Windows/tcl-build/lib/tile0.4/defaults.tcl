#
# $Id: defaults.tcl,v 1.12 2004/04/06 02:19:46 jenglish Exp $
#
# Tile widget set: Default theme
#

namespace eval tile {
style theme settings default {

    style default "." \
    	-background $defaultBG \
	-foreground black \
	-troughcolor $troughColor \
	-font ButtonFont \
        -highlightthickness 1 \
	;

    style map "." -background [list disabled $defaultBG  active $activeBG]
    style map "." -foreground [list disabled #a3a3a3]
    style map "." -highlightcolor [list focus black {} $defaultBG]

    style default TButton -padding "3m 1m" -relief raised -shiftrelief 1
    style map TButton \
    	-relief [list {!disabled pressed} sunken] \

    foreach class {TCheckbutton TRadiobutton} {
	style map $class -indicatorrelief \
	    [list  selected sunken   pressed sunken  {} raised] \
	-indicatorcolor \
	    [list  pressed $defaultBG  selected #b03060  {} $defaultBG] \
    }

    style default TMenubutton -relief raised -padding "3m 1m"

    style map TScrollbar -relief { pressed sunken  {} raised }
    style map TScale -sliderrelief {{pressed !disabled} sunken  {} raised}
    style map TProgress -background {{} SteelBlue}

    style default Tab.TNotebook -padding {3m 1m}
    style map Tab.TNotebook -background \
    	[list selected $defaultBG {} $troughColor]
    style map TNotebook -bordercolor {focus SteelBlue}

    #
    # Toolbar buttons:
    #
    style layout Toolbutton {
        Toolbutton.background
        Toolbutton.border -children {
            Toolbutton.padding -children {
                Toolbutton.label
            }
        }
    }

    style default Toolbutton -padding 2 -relief flat
    style map Toolbutton -relief \
    	{disabled flat selected sunken pressed sunken active raised} ;
    style map Toolbutton -background \
    	[list pressed $troughColor  active $activeBG] ;

}}

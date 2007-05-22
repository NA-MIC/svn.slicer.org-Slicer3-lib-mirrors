#
# $Id: altTheme.tcl,v 1.14 2004/04/06 02:19:46 jenglish Exp $
#
# Tile widget set: Alternate theme
#

namespace eval tile {

style theme settings alt {

    style default "." \
    	-background $defaultBG \
	-foreground black \
	-troughcolor $troughColor \
	-font ButtonFont \
	;

    style map "." \
    	-background [list disabled $defaultBG  active $activeBG] \
	-foreground [list disabled #a3a3a3] \
	;

    style default TButton \
    	-width -11 -padding "1 1" -relief raised -shiftrelief 1 \
    	-highlightthickness 1 -highlightcolor $defaultBG

    style map TButton -relief {
	{pressed !disabled} 	sunken
	{active !disabled} 	raised
    } -highlightcolor {default black}

    style map TCheckbutton -indicatorcolor \
	[list  disabled $defaultBG  pressed $defaultBG  {} #FFFFFF]
    style map TRadiobutton -indicatorcolor \
	[list  disabled $defaultBG  pressed $defaultBG  {} #FFFFFF]

    style default TMenubutton -width 11 -padding "3 3" -relief raised

    style default Toolbutton -relief flat -padding 2
    style map Toolbutton -relief \
    	{disabled flat selected sunken pressed sunken active raised} ;
    style map Toolbutton -background \
    	[list pressed $troughColor  active $activeBG] ;

    style map TScrollbar -relief { pressed sunken  {} raised }

    style default Tab.TNotebook -padding {10 3 10 3}
    style map Tab.TNotebook \
	-padding [list selected {10 5 10 3}] \
    	-background [list selected $defaultBG {} $troughColor] \
	;

    style default TScale -groovewidth 4 -troughrelief sunken
    style map TScale -sliderrelief {pressed sunken  {} raised}
    style default TProgress -background SteelBlue

} }

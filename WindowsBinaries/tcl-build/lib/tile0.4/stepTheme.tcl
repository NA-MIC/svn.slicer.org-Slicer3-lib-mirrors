#
# $Id: stepTheme.tcl,v 1.15 2004/04/06 02:19:46 jenglish Exp $
#
# Appearance settings for "Step" theme.
#

namespace eval tile {

style theme settings step {

    set step_defaultBG $defaultBG ;#"#a0a0a0"
    set step_lightBG   $defaultBG
    set step_activeBG  $activeBG  ;#"#d0d0d0"
    set step_scrollBG  $defaultBG ;#"#aeb2c3"

    style default "." \
    	-background $defaultBG \
	-foreground black \
	-troughcolor $troughColor \
	-font "Helvetica 12" \
	;

    style map "." \
    	-background [list disabled $defaultBG  active $step_activeBG] \
	-foreground [list disabled #a3a3a3] \
	;

    style default TButton -padding "3m 0" -relief raised -shiftrelief 1
    style map TButton -relief {
	{pressed !disabled} 	sunken
	{active !disabled} 	raised
    }

    style default TCheckbutton -indicatorrelief raised
    style map TCheckbutton  -indicatorrelief [list pressed sunken]

    style default TRadiobutton \
    	-indicatorcolor $step_lightBG \
    	-indicatorrelief groove ;
    style map TRadiobutton -indicatorrelief [list pressed sunken] 

    style default TMenubutton -width 11 -padding "3 3" -relief raised

    style map TScrollbar -relief { pressed sunken  {} raised }
    style map TScrollbar -background \
    	[list  disabled $step_defaultBG  active $step_scrollBG] ;

    style default TProgress -background SteelBlue

    style default Tab.TNotebook -padding {10 3}
    style map Tab.TNotebook \
	-background \
	    [list selected $defaultBG active $activeBG {} $troughColor] \
	-padding [list selected {12 6 12 3}]

} }


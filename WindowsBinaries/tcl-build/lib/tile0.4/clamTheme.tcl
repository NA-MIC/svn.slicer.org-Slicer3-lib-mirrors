#
# $Id: clamTheme.tcl,v 1.3 2004/04/25 14:24:16 jenglish Exp $
#
# Tile widget set: "Clam" theme
#
# Inspired by the XFCE family of Gnome themes.
#

namespace eval tile::theme::clam {

package provide tile::theme::clam 0.0.1

variable colors ; array set colors {
    -disabledfg "#999999"

    -frame	"#dcdad5"
    -dark  	"#cfcdc8"
    -darker 	"#bab5ab"
    -darkest	"#9e9a91"
    -lighter	"#eeebe7"
    -lightest 	"#ffffff"
}

style theme settings clam {

    style default "." \
    	-background $colors(-frame) \
	-foreground black \
    	-bordercolor $colors(-darkest) \
    	-darkcolor $colors(-dark) \
    	-lightcolor $colors(-lightest) \
	-troughcolor $colors(-darker) \
	-font "Helvetica -12" \
	;

    style map "." \
    	-background [list disabled $colors(-frame)  active $colors(-lighter)] \
	-foreground [list disabled $colors(-disabledfg)] \
	;

    style default TButton -width -11 -padding 5 -relief raised
    style map TButton \
    	-background [list \
	    disabled $colors(-frame) \
	    pressed $colors(-darker) \
	    active $colors(-lighter)] \
    	-lightcolor [list pressed $colors(-darker)] \
    	-darkcolor [list pressed $colors(-darker)] \
	-bordercolor [list default #000000] \
    ;

    style default Toolbutton -padding 2 -relief flat
    style map Toolbutton \
	-relief {disabled flat selected sunken pressed sunken active raised} \
    	-background [list \
	    disabled $colors(-frame) \
	    pressed $colors(-darker) \
	    active $colors(-lighter)] \
    	-lightcolor [list pressed $colors(-darker)] \
    	-darkcolor [list pressed $colors(-darker)] \
    ;

    style default TCheckbutton \
    	-indicatorbackground #ffffff -indicatormargin {1 1 4 1}
    style default TRadiobutton \
    	-indicatorbackground #ffffff -indicatormargin {1 1 4 1}
    style map TCheckbutton -indicatorbackground \
	[list  disabled $colors(-frame)  pressed $colors(-frame)]
    style map TRadiobutton -indicatorbackground \
	[list  disabled $colors(-frame)  pressed $colors(-frame)]

    style default TMenubutton -width 11 -padding 3 -relief raised

    style default Tab.TNotebook -padding {10 3 10 3}
    style map Tab.TNotebook \
	-padding [list selected {10 5 10 3}] \
    	-background [list selected $colors(-frame) {} $colors(-darker)] \
    	-lightcolor [list selected $colors(-lightest) {} $colors(-dark)] \
	;

    style default TScale -groovewidth 4 -troughrelief sunken
    style map TScale -sliderrelief {pressed sunken  {} raised}
    style default TProgress -background SteelBlue
} }


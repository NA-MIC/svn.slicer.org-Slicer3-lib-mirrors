# blue.tcl - Copyright (C) 2004 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# A sample pixmap theme for the tile package.
#
# $Id: blue.tcl,v 1.19 2004/04/27 01:01:14 patthoyts Exp $

package require tile::pixmap

namespace eval tile {
    namespace eval blue {
        variable version 0.0.1
    }
}

namespace eval tile::blue {

set imgdir [file join [file dirname [info script]] blue]
array set Images [tile::LoadImages $imgdir *.gif]

style theme create blue -parent alt -settings {

    style default . -background #6699CC -troughcolor #bcd2e8 
    style default . -font ButtonFont -borderwidth 1
    #style map . -background [list active #bcd2e8]
    style map . -foreground [list disabled #a3a3a3]
    style default TButton -padding "10 0"

    style default Tab.TNotebook -padding {10 3}
    style map Tab.TNotebook \
	-background \
	    [list selected #6699CC active #bcd2e8] \
	-padding [list selected {12 6 12 3}]

    style default TScrollbar -width 12
    style map TScrollbar \
    	-relief		{ pressed sunken } \
	-arrowcolor	{ pressed #00CC00 } \
	;

    style layout Vertical.TScrollbar {
    	Scrollbar.background 
	Scrollbar.trough -children {
	    Scrollbar.uparrow -side top
	    Scrollbar.downarrow -side bottom
	    Scrollbar.uparrow -side bottom
	    Vertical.Scrollbar.thumb -side top -expand true -sticky ns
	}
    }

    style layout Horizontal.TScrollbar {
    	Scrollbar.background 
	Scrollbar.trough -children {
	    Scrollbar.leftarrow -side left
	    Scrollbar.rightarrow -side right
	    Scrollbar.leftarrow -side right
	    Horizontal.Scrollbar.thumb -side left -expand true -sticky we
	}
    }

    #
    # Elements:
    #
    style element create Button.background pixmap -images [list  \
    	pressed $Images(button-p) \
	active	$Images(button-h) \
        {} $Images(button-n) \
    ] -border 4 -tiling tile

    style element create Checkbutton.indicator pixmap -images [list \
        {active selected} 	$Images(check-hc) \
        active			$Images(check-hu) \
        selected		$Images(check-nc) \
        {}                 	$Images(check-nu) \
    ] -tiling fixed

    style element create Radiobutton.indicator pixmap -images [list \
        {active selected} 	$Images(radio-hc) \
        active			$Images(radio-hu) \
        selected		$Images(radio-nc) \
        {}                 	$Images(radio-nu) \
    ] -tiling fixed

    style element create Horizontal.Scrollbar.thumb pixmap -images [list \
        {pressed !disabled} $Images(sb-thumb-p) \
        {} $Images(sb-thumb) \
    ] -tiling tile -border 3

    style element create Vertical.Scrollbar.thumb pixmap -images [list \
        {pressed !disabled} $Images(sb-vthumb-p) \
        {} $Images(sb-vthumb) \
    ] -tiling tile -border 3

    style element create Scale.slider pixmap -images [list \
        {pressed !disabled} $Images(slider-p) \
        {} $Images(slider) \
    ] -tiling tile -border 3

    style element create Vertical.Scale.slider pixmap -images [list \
        {pressed !disabled} $Images(vslider-p) \
        {} $Images(vslider) \
    ] -tiling tile -border 3

    style element create Horizontal.Progress.bar pixmap -images [list \
        {} $Images(sb-thumb) \
    ] -tiling tile -border 3
      
    style element create Vertical.Progress.bar pixmap -images [list \
        {} $Images(sb-vthumb) \
    ] -tiling tile -border 3
      
    style element create Scrollbar.uparrow pixmap -images [list \
        {pressed !disabled} $Images(arrowup-p) \
        {active !disabled} $Images(arrowup-h) \
        {} $Images(arrowup) \
    ] -tiling fixed
    style element create Scrollbar.downarrow pixmap -images [list \
        {pressed !disabled} $Images(arrowdown-p) \
        {active !disabled} $Images(arrowdown-h) \
        {} $Images(arrowdown) \
    ] -tiling fixed
    style element create Scrollbar.rightarrow pixmap -images [list \
        {pressed !disabled} $Images(arrowright-p) \
        {active !disabled} $Images(arrowright-h) \
        {} $Images(arrowright) \
    ] -tiling fixed
    style element create Scrollbar.leftarrow pixmap -images [list \
        {pressed !disabled} $Images(arrowleft-p) \
        {active !disabled} $Images(arrowleft-h) \
        {} $Images(arrowleft) \
    ] -tiling fixed

    #
    # Settings:
    #
    style layout TButton {
    	Button.background
	Button.focus -children {
	    Button.padding -children {
	    	Button.label -side left -expand true
	    }
	}
    }

    style layout TCheckbutton {
        Checkbutton.background
        Checkbutton.focus -children {
            Checkbutton.border -children {
                Checkbutton.padding -children {
                    Checkbutton.indicator -side right
                    Checkbutton.label -side left -expand true
                }
            }
        }
    }

    style layout TRadiobutton {
        Radiobutton.background
	Radiobutton.border -children {
	    Radiobutton.padding -children  {
		Radiobutton.indicator -side right
		Radiobutton.focus -expand true -sticky {} -children {
		    Radiobutton.label -side right -expand true
		}
	    }
	}
    }

} }

# -------------------------------------------------------------------------

package provide tile::theme::blue $::tile::blue::version

# -------------------------------------------------------------------------

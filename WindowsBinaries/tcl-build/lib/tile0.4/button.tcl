#
# $Id: button.tcl,v 1.3 2004/03/20 15:30:27 jenglish Exp $
#
# Bindings for Buttons, Checkbuttons, and Radiobuttons:
#
# Notes: <Button1-Leave>, <Button1-Enter> only control the "pressed"
# state; widgets remain "active" if the pointer is dragged out.
# This doesn't seem to be conventional, but it's a nice way
# to provide extra feedback while the grab is active.
# (If the button is released off the widget, the grab deactivates and
# we get a <Leave> event then, which turns off the "active" state)
#

namespace eval tile {
foreach class {TButton TCheckbutton TRadiobutton} {
    bind $class <Enter>		{ %W state active }
    bind $class <Leave>		{ %W state !active }
    bind $class <Key-space>	{ tile::activate %W }
    bind $class <<Invoke>> 	{ tile::activate %W }

    bind $class <ButtonPress-1>	\
    	{ %W instate !disabled { focus %W; %W state pressed } }
    bind $class <ButtonRelease-1> \
	{ %W instate {pressed !disabled} { %W state !pressed ; %W invoke } }
    bind $class <Button1-Leave>	{ %W instate !disabled { %W state !pressed } }
    bind $class <Button1-Enter>	{ %W instate !disabled { %W state pressed } }

    proc activate {w} {
	set oldState [$w state pressed]
	update idletasks; after 100
	$w state $oldState
	after idle [list $w invoke]
    }
}

# bind TCheckbutton <KeyPress-plus> { %W select }
# bind TCheckbutton <KeyPress-minus> { %W deselect }

}


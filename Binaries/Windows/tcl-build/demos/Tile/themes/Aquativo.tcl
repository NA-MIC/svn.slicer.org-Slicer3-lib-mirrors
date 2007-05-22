# Aquativo - Copyright (C) 2004 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# Import the Aquativo Gtk theme (C) Andrew Wyatt, FEWT Software
# Original: http://www.fewt.com
# Link: http://art.gnome.org/themes/gtk2/432.php
#
# You will need to fetch the theme package and extract it under the 
# demos/themes directory and maybe modify the demos/themes/pkgIndex.tcl
# file.
#
# $Id: Aquativo.tcl,v 1.16 2004/04/06 02:19:45 jenglish Exp $

package require tile::pixmap
package require Img

namespace eval tile {
    namespace eval Aquativo {
        variable version 0.0.1
    }
}

namespace eval tile::Aquativo {

style theme create Aquativo -parent alt -settings {

    # ---------------------------------------------------------------------
    # Layouts
    # ---------------------------------------------------------------------

    style layout TButton {
        Button.background
	Button.padding -children {
	    Button.focus -expand true -children {
		Button.label
	    }
        }
    }

    # ---------------------------------------------------------------------
    # Troughs
    # ---------------------------------------------------------------------

    style element create trough pixmap -images {
        {}                  ::tile::Aquativo::_i_horizontal_trough
    } -border 3 -tiling tile -background opaque
    style element create Vertical.Scrollbar.trough pixmap -images {
        {}                  ::tile::Aquativo::_i_vertical_trough
    } -border 2 -tiling tile -background opaque
    style element create Vertical.Scale.trough pixmap -images {
        {}                  ::tile::Aquativo::_i_vertical_trough
    } -border 2 -tiling tile -background opaque
    style element create Vertical.Progress.trough pixmap -images {
        {}                  ::tile::Aquativo::_i_vertical_trough
    } -border 2 -tiling tile -background opaque


    # ---------------------------------------------------------------------
    # Buttons, Checkbuttons and Radiobuttons
    # ---------------------------------------------------------------------

    style element create Button.background pixmap -images {
        {pressed !disabled} ::tile::Aquativo::_i_button_normal_in
        {active !disabled}  ::tile::Aquativo::_i_button_prelight_out
        disabled            ::tile::Aquativo::_i_button_insensitive_out
        {}                  ::tile::Aquativo::_i_button_normal_out
    } -border 10 -tiling tile -background opaque

    style element create Checkbutton.indicator pixmap -images {
        {selected !disabled}  ::tile::Aquativo::_i_check_in
        {}                    ::tile::Aquativo::_i_check_out
    } -border 2 -tiling fixed

    style element create Radiobutton.indicator pixmap -images {
        {selected !disabled}  ::tile::Aquativo::_i_option_in
        {}                    ::tile::Aquativo::_i_option_out
    } -border 2 -tiling fixed

    # ---------------------------------------------------------------------
    # Menubuttons:
    # ---------------------------------------------------------------------
    style element create Menubutton.button pixmap -images {
	{} ::tile::Aquativo::_i_menubar_option
    } -tiling fixed
    # @@@ Need asymmetric borders to draw this properly...

    style element create Menubutton.indicator pixmap -images {
	{} ::tile::Aquativo::_i_menubar_option_arrow
    } -tiling fixed

    if {0} {
    style layout TMenubutton {
	Menubutton.background
	Menubutton.button -expand true -sticky nwse -children {
	    Menubutton.indicator -side right
	    Menubutton.label -side left -expand true
	}
    }
    }

    # ---------------------------------------------------------------------
    # Scrollbar
    # ---------------------------------------------------------------------

    style element create Horizontal.Scrollbar.thumb pixmap -images {
        {active !disabled}  ::tile::Aquativo::_i_scrollbar_prelight_horizontal
        {}                  ::tile::Aquativo::_i_scrollbar_horizontal
    } -border 3 -tiling tile -minsize {8 0}

    style element create Vertical.Scrollbar.thumb pixmap -images {
        {active !disabled}  ::tile::Aquativo::_i_scrollbar_prelight_vertical
        {}                  ::tile::Aquativo::_i_scrollbar_vertical
    } -border 3 -tiling tile -minsize {0 8}

    # ---------------------------------------------------------------------
    # Scale
    # ---------------------------------------------------------------------

    style element create Horizontal.Scale.slider pixmap -images {
        {active !disabled}  ::tile::Aquativo::_i_scrollbar_prelight_horizontal
        {}                  ::tile::Aquativo::_i_scrollbar_horizontal
    } -border 3 -tiling tile -minsize {30 0}

    style element create Vertical.Scale.slider pixmap -images {
        {active !disabled}  ::tile::Aquativo::_i_scrollbar_prelight_vertical
        {}                  ::tile::Aquativo::_i_scrollbar_vertical
    } -border 3 -tiling tile -minsize {0 30}

    # ---------------------------------------------------------------------
    # Progress
    # ---------------------------------------------------------------------

    # should use ::tile::Aquativo::_i_progressbar but this is too big.
    style element create bar pixmap -images {
        {} ::tile::Aquativo::_i_scrollbar_prelight_horizontal
    } -border 3 -tiling tile
    style element create Vertical.Progress.bar pixmap -images {
        {} ::tile::Aquativo::_i_scrollbar_prelight_vertical
    } -border 3 -tiling tile

    # ---------------------------------------------------------------------
    # Arrows
    # ---------------------------------------------------------------------

    style element create uparrow pixmap -images {
        {pressed !disabled} ::tile::Aquativo::_i_arrow_up_clicked
        disabled            ::tile::Aquativo::_i_arrow_up_insensitive
        {}                  ::tile::Aquativo::_i_arrow_up_normal
    } -tiling fixed
        
    style element create downarrow pixmap -images {
        {pressed !disabled} ::tile::Aquativo::_i_arrow_down_clicked
        disabled            ::tile::Aquativo::_i_arrow_down_insensitive
        {}                  ::tile::Aquativo::_i_arrow_down_normal
    } -tiling fixed

    style element create leftarrow pixmap -images {
        {pressed !disabled} ::tile::Aquativo::_i_arrow_left_clicked
        disabled            ::tile::Aquativo::_i_arrow_left_insensitive
        {}                  ::tile::Aquativo::_i_arrow_left_normal
    } -tiling fixed

    style element create rightarrow pixmap -images {
        {pressed !disabled} ::tile::Aquativo::_i_arrow_right_clicked
        disabled            ::tile::Aquativo::_i_arrow_right_insensitive
        {}                  ::tile::Aquativo::_i_arrow_right_normal
    } -tiling fixed

    # ---------------------------------------------------------------------
    # Notebook parts
    # ---------------------------------------------------------------------
    
    style element create tab pixmap -images {
        {selected !disabled} ::tile::Aquativo::_i_notebook_bottom
        {}                   ::tile::Aquativo::_i_notebook_active_bottom
    } -tiling fixed

    # ---------------------------------------------------------------------
    # Frames parts
    # ---------------------------------------------------------------------

    # This make it REALLY SLOW
    #style element create Frame.background pixmap -images {
    #    {}                   ::tile::Aquativo::_i_window_background
    #} -tiling tile
    #style element create Labelframe.background pixmap -images {
    #    {}                   ::tile::Aquativo::_i_window_background
    #} -tiling tile

    # ---------------------------------------------------------------------
    # Load images
    # ---------------------------------------------------------------------

    set imgdir [file join [file dirname [info script]] Aquativo-1.5 gtk-2.0]
    foreach file [glob -directory $imgdir *.png] {
        set img [file tail [file rootname $file]]
        image create photo [namespace current]::_i_$img -file $file
    }

    style default . -background #fafafa -font ButtonFont
    style default TButton -padding "3 3" -width -11

} }

# -------------------------------------------------------------------------

package provide tile::theme::Aquativo $::tile::Aquativo::version

# -------------------------------------------------------------------------

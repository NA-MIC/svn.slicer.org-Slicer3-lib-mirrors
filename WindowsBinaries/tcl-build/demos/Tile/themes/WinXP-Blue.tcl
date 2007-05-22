# WinXP-Blue - Copyright (C) 2004 Pat Thoyts <patthoyts@users.sourceforge.net>
#
# Import the WinXP-Blue Gtk2 Theme by Ativo
# Link:
# URL: http://art.gnome.org/download/themes/gtk2/474/GTK2-WinXP-Blue.tar.gz
#
# You will need to fetch the theme package and extract it under the
# demos/themes directory and maybe modify the demos/themes/pkgIndex.tcl
# file.
#
# $Id: WinXP-Blue.tcl,v 1.13 2004/04/06 02:19:45 jenglish Exp $

package require tile::pixmap
package require Img

namespace eval tile {
    namespace eval WinXPBlue {
        variable version 0.0.1
    }
}

namespace eval tile::WinXPBlue {

set imgdir [file join [file dirname [info script]] WinXP-Blue gtk-2.0]
array set Images [tile::LoadImages $imgdir *.png]

style theme create WinXPBlue -parent alt -settings {

    style default TButton -padding "3 3" -width -11
    style default "." -background #ece9d8 -font ButtonFont

    style map "." -foreground {
	disabled	#565248
    } -background {
        disabled	#e3e1dd
	pressed		#bab5ab
    }

    style layout TButton {
        Button.background
	Button.padding -children {
	    Button.focus -expand true -children {
		Button.label
	    }
        }
    }

    style element create Checkbutton.indicator pixmap -images [list \
        selected $Images(checkbox_checked) \
        {}       $Images(checkbox_unchecked) \
    ] -tiling fixed

    style element create Radiobutton.indicator pixmap -images [list \
        selected $Images(option_in) \
        {}       $Images(option_out) \
    ] -tiling fixed

    style element create Horizontal.Scrollbar.thumb pixmap -images [list \
        {}      $Images(scroll_horizontal) \
    ] -border 3 -tiling tile -minsize {8 0}

    style element create Vertical.Scrollbar.thumb pixmap -images [list \
        {}      $Images(scroll_vertical) \
    ] -border 3 -tiling tile -minsize {0 8}

    style element create trough pixmap -images [list \
        {}      $Images(horizontal_trough) \
    ] -tiling tile -background opaque

    style element create Vertical.Scrollbar.trough pixmap -images [list \
        {}      $Images(vertical_trough) \
    ] -tiling tile -background opaque
    style element create Vertical.Scale.trough pixmap -images [list \
        {}      $Images(vertical_trough) \
    ] -tiling tile -background opaque

    # Progress
    style element create Progress.bar pixmap -images [list \
        {}      $Images(progressbar) \
    ] -border 0 -tiling tile
    style element create Vertical.Progress.bar pixmap -images [list \
        {}      $Images(progressbar) \
    ] -border 0 -tiling tile
    style element create Progress.trough pixmap -images [list \
        {}      $Images(through) \
    ] -border 4 -tiling tile -background opaque

    # Button
    style element create Button.background pixmap -images [list \
        {pressed !disabled} $Images(buttonPressed) \
        {}                  $Images(buttonNorm) \
    ] -border 4 -tiling tile -background opaque

    # ---------------------------------------------------------------------
    # Notebook parts
    # ---------------------------------------------------------------------

    style element create tab pixmap -images [list \
        {selected !disabled} $Images(notebook_active) \
        {}                   $Images(notebook_inactive) \
    ] -border 6 -tiling tile

    # ---------------------------------------------------------------------
    # Arrows
    # ---------------------------------------------------------------------

    style element create uparrow pixmap -images [list \
        pressed $Images(arrow_up_clicked) \
        {}      $Images(arrow_up_normal) \
    ] -tiling fixed

    style element create downarrow pixmap -images [list \
        pressed $Images(arrow_down_clicked) \
        {}      $Images(arrow_down_normal) \
    ] -tiling fixed

    style element create leftarrow pixmap -images [list \
        pressed $Images(arrow_left_clicked) \
        {}      $Images(arrow_left_normal) \
    ] -tiling fixed

    style element create rightarrow pixmap -images [list \
        pressed $Images(arrow_right_clicked) \
        {}      $Images(arrow_right_normal) \
    ] -tiling fixed

} }

# -------------------------------------------------------------------------

package provide tile::theme::WinXPBlue $::tile::WinXPBlue::version

# -------------------------------------------------------------------------

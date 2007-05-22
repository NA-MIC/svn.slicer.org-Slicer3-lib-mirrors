#
# $Id: notebook.tcl,v 1.6 2004/04/23 02:21:43 jenglish Exp $
# 
# Bindings for TNotebook widget
#

bind TNotebook <ButtonPress-1>		{ tile::notebook::Select %W %x %y }
bind TNotebook <Key-Right>		{ tile::notebook::CycleTab %W 1 }
bind TNotebook <Key-Left>		{ tile::notebook::CycleTab %W -1 }
bind TNotebook <Control-Key-Tab>	{ tile::notebook::CycleTab %W 1 }
bind TNotebook <Control-Shift-Key-Tab>	{ tile::notebook::CycleTab %W -1 }

# @@@ BUG: Control-Shift-Key-Tab bindings never fire, 
# @@@ since the <<PrevWindow>> virtual binding takes precedence.

namespace eval tile {
    namespace eval notebook {

	# Select $nb $x $y --
	#	ButtonPress-1 binding for notebook widgets.
	#	If mouse cursor is over a tab, select it.
	#
    	proc Select {w x y} {
	    set tab [$w index @$x,$y]
	    if {$tab ne ""} {
	    	$w select $tab
		focus $w
		event generate $w <<NotebookTabChanged>>
	    }
	}

	# CycleTab --
	#	Select the next/previous tab in the list.
	#	Only call this from a binding script;
	#	it returns a TCL_BREAK code.
	#
	proc CycleTab {w dir} {
	    if {[$w index end] == 0} { return }
	    $w select [expr {([$w index current] + $dir) % [$w index end]}]
	    event generate $w <<NotebookTabChanged>>
	    return -code break
	}

	proc MnemonicActivation {nb key} {
	    set key [string toupper $key]
	    foreach tab [$nb tabs] {
		set label [$nb tabcget $tab -text]
		set underline [$nb tabcget $tab -underline]
		set mnemonic [string toupper [string index $label $underline]]
		if {$mnemonic ne "" && $mnemonic eq $key} {
		    $nb select $tab
		    event generate $nb <<NotebookTabChanged>>
		    # @@@ SHOULD: set focus here.
		    return -code break
		}
	    }
	}

	# enableTraversal --
	#	Enable keyboard traversal for a notebook widget
	#	by adding bindings to the containing toplevel window.
	#
    	proc enableTraversal {nb} {
	    set top [winfo toplevel $nb]
	    bind $top <Control-Key-Tab> \
	    	+[list tile::notebook::CycleTab $nb 1]
	    bind $top <Shift-Control-Key-Tab> \
	    	+[list tile::notebook::CycleTab $nb -1]
	    bind $top <Alt-KeyPress> \
	    	+[list tile::notebook::MnemonicActivation $nb %K]
	}
    }
}


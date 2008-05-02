#!../src/bltwish

if { [info exists env(BLT_LIBRARY)] } {
   lappend auto_path $env(BLT_LIBRARY)
}
package require BLT

source scripts/demo.tcl

set normalBg [blt::bgstyle create texture -low grey85 -high grey88]
set activeBg [blt::bgstyle create gradient -low grey40 -high grey95 \
	-jitter yes -log no]
set normalBg white
set activeBg grey95
option add *Axis.activeBackground $activeBg
option add *Legend.activeBackground $activeBg

set graph .g
blt::graph .g \
    -bg $normalBg \
    -plotrelief solid \
    -plotborderwidth 0 \
    -relief solid \
    -plotpadx 0 -plotpady 0 \
    -borderwidth 0

blt::htext .header \
    -text {\
This is an example of the graph widget.  It displays two-variable data 
with assorted line attributes and symbols.  To create a postscript file 
"xy.ps", press the %%
    blt::tile::button $htext(widget).print -text print -command {
        puts stderr [time {
	   blt::busy hold .
	   update
	   .g postscript output demo1.eps 
	   update
	   blt::busy release .
	   update
        }]
    } 
    $htext(widget) append $htext(widget).print
%% button.}

source scripts/graph1.tcl

blt::htext .footer \
    -text {Hit the %%
blt::tile::button $htext(widget).quit -text quit -command { exit } 
$htext(widget) append $htext(widget).quit 
%% button when you've seen enough.%%
label $htext(widget).logo -bitmap BLT
$htext(widget) append $htext(widget).logo 
%%}

proc MultiplexView { args } { 
    eval .g axis view y $args
    eval .g axis view y2 $args
}

blt::tile::scrollbar .xbar \
    -command { .g axis view x } \
    -orient horizontal \
    -highlightthickness 0
blt::tile::scrollbar .ybar \
    -command MultiplexView \
    -orient vertical -highlightthickness 0
blt::table . \
    0,0 .header -cspan 3 -fill x \
    1,0 .g  -fill both -cspan 3 -rspan 3 \
    2,3 .ybar -fill y  -padx 0 -pady 0 \
    4,1 .xbar -fill x \
    5,0 .footer -cspan 3 -fill x

blt::table configure . c3 r0 r4 r5 -resize none

.g postscript configure \
    -center yes \
    -maxpect yes \
    -landscape yes \
    -preview ""

.g axis configure x \
    -scrollcommand { .xbar set }  \
    -scrollmax 10 \
    -scrollmin 2  \
    -title "X" 

.g axis configure y \
    -scrollcommand { .ybar set } \
    -rotate 90 \
    -title "Y" 

.g axis configure y2 \
    -scrollmin 0.0 -scrollmax 1.0 \
    -hide yes \
    -rotate -90 \
    -title "Y2"

.g legend configure \
    -relief flat -bd 0 \
    -activerelief flat \
    -activeborderwidth 1  \
    -position right -anchor ne -bg ""

#.g configure -plotpadx 0 -plotpady 0 -plotborderwidth 0

.g pen configure "activeLine" \
    -showvalues y

.g element bind all <Enter> {
    %W legend activate [%W element get current]
}
.g configure -plotpady { 0 0 } 

.g element bind all <Leave> {
    %W legend deactivate [%W element get current]
}
.g axis bind all <Enter> {
    %W axis activate [%W axis get current]
    %W axis focus [%W axis get current] 
}
.g axis bind all <Leave> {
    %W axis deactivate [%W axis get current]
    %W axis focus ""
}
.g configure -leftvariable left 
trace variable left w "UpdateTable .g"
proc UpdateTable { graph p1 p2 how } {
    blt::table configure . c0 -width [$graph extents leftmargin]
    blt::table configure . c2 -width [$graph extents rightmargin]
    blt::table configure . r1 -height [$graph extents topmargin]
    blt::table configure . r3 -height [$graph extents bottommargin]
}

#set image2 [image create picture -file images/blt98.gif]
set image2 [image create picture -file images/buckskin.gif]
.g element configure line2 -areapattern @bitmaps/sharky.xbm \
	-areaforeground blue -areabackground ""
.g element configure line3
# -areatile $image2
.g configure -title "Graph Title"

if { $tcl_platform(platform) == "windows" } {
    if 0 {
        set name [lindex [blt::printer names] 0]
        set printer {Lexmark Optra E310}
	blt::printer open $printer
	blt::printer getattrs $printer attrs
	puts $attrs(Orientation)
	set attrs(Orientation) Landscape
	set attrs(DocumentName) "This is my print job"
	blt::printer setattrs $printer attrs
	blt::printer getattrs $printer attrs
	puts $attrs(Orientation)
	after 5000 {
	    $graph print2 $printer
	    blt::printer close $printer
	}
    } else {
	after 5000 {
	    $graph print2 
	}
    }	
    if 1 {
	after 2000 {$graph snap -format emf CLIPBOARD}
    }
}

focus .g
.g axis bind all <Left>  { 
    set axis [%W axis get current] 
    .g axis view $axis scroll -1 units 
} 

.g axis bind all <Right> { 
    set axis [%W axis get current] 
    .g axis view $axis scroll 1 units 
}


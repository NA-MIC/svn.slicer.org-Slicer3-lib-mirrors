#!../src/bltwish

package require BLT
source scripts/demo.tcl

image create picture bgTile -file ./images/smblue_rock.gif
image create picture label1 -file ./images/mini-book1.gif
image create picture label2 -file ./images/mini-book2.gif
image create picture testImage -file ./images/txtrflag.gif

scrollbar .s -command { .t view } -orient horizontal
blt::tabset .t \
    -relief flat \
    -bd 2 \
    -textside right \
    -samewidth yes \
    -tiers 2 \
    -slant right \
    -scrollcommand { .s set } \
    -tile bgTile \
    -scrollincrement 1  -selectforeground green2

label .t.l -image testImage

set attributes {
    graph1 "Graph \#1" red	.t.graph1  
    graph2 "Graph \#2" green	.t.graph2  
    graph3 "Graph \#3" cyan	.t.graph3  
    graph5 "Graph \#5" yellow	.t.graph5  
    graph6 one		orange	.t.l       
}

foreach { entry label color window } $attributes {
    .t insert end $entry -text $label -fill both
}

.t insert end \
    there bunky another test of \
    a -image label2 widget

blt::table . \
    .t 0,0 -fill both \
    .s 1,0 -fill x 

blt::table configure . r1 -resize none

foreach file { graph1 graph2 graph3 graph5 } {
    namespace eval $file {
	set graph [blt::graph .t.$file]
	source scripts/$file.tcl
	.t tab configure $file -window $graph
    }
}


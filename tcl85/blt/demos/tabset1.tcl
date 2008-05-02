#!../src/bltwish

package require BLT
source scripts/demo.tcl

image create picture bgTile -file ./images/chalk.gif
image create picture label1 -file ./images/mini-book1.gif
image create picture label2 -file ./images/mini-book2.gif

blt::tabset .t \
    -relief raised \
    -activebackground yellow \
    -bg red \
    -borderwidth 0 \
    -highlightthickness 0 \
    -scrollcommand { .s set } \
    -width 3i 

#option add *iPadX 4
#option add *iPadY 2

.t insert end First \
    -image label1 \
    -anchor center \
    -selectbackground darkolivegreen2  \
    Again Next another test of \
    a -image label2 widget 

scrollbar .s -command { .t view } -orient horizontal
blt::table . \
    .t 0,0 -fill both \
    .s 1,0 -fill x 

blt::table configure . r1 -resize none
focus .t


#!../src/bltwish

package require BLT
source scripts/demo.tcl
#blt::bltdebug 100

source scripts/stipples.tcl

blt::tabset .t \
    -samewidth yes \
    -side left \
    -textside bottom \
    -textside top \
    -bg red \
    -tiers 1 \
    -scrollincrement 10 \
    -scrollcommand { .s set } \
    -rotate 0 \
    -selectcommand {  MakePicture %W %n } 


scrollbar .s -command { .t view } -orient horizontal
 
option clear
option add *Tabset.Tab.font -*-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*

set files [glob ./images/*.gif]
set files [lsort $files]
set vertFilter sinc
set horzFilter sinc
#set vertFilter none
#set horzFilter none


proc ResizePicture { src dest maxSize } {
    puts stderr "maxSize=$maxSize"
    set maxSize [winfo fpixels . $maxSize]
    set w [image width $src]
    set h [image height $src]
    puts stderr "width=$w, height=$h"
    set sw [expr double($maxSize) / $w]
    set sh [expr double($maxSize) / $h]
    puts stderr "sw=$sw,sh=$sh"
    set s [expr min($sw, $sh)]
    set w [expr round($s * $w)]
    set h [expr round($s * $h)]
    puts stderr "[$src configure]"
    $dest configure -width $w -height $h
    
    global horzFilter vertFilter
    $dest resize $src -filter $horzFilter 
}

image create picture src
image create picture dest

label .t.label -image dest -bg purple

proc MakePicture { w name } {
    set file ./images/$name.gif
    src configure -file $file

    set width [$w tab pagewidth]
    set height [$w tab pageheight]
    puts stderr "pagewidth=$width, pageheight=$height"
    if { $width < $height } {
	ResizePicture src dest $width
    } else {
	ResizePicture src dest $height
    }
    .t tab dockall
    .t tab configure $name -window .t.label -padx 4m -pady 4m -fill both
}

blt::table . \
    .t 0,0 -fill both \
    .s 1,0 -fill x 

blt::table configure . r1 -resize none
focus .t

foreach f $files {
    src configure -file $f
    set f [file tail [file root $f]]
    set thumb [image create picture]
    ResizePicture src $thumb 0.5i
    .t insert end $f -image $thumb -fill both
}

.t focus 0
.t invoke 0

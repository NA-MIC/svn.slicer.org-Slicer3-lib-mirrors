
set dest [image create picture -width 600 -height 400]

source usmap.tcl

set count 0
array set colors {
    0 red 
    1 green 
    2 blue 
    3 cyan 
    4 orange 
    5 purple
    6 brown
    7 violet
    8 seagreen
    9 lightblue
    10 yellow
    11 pink
    12 khaki
    13 grey
}

blt::vector all
blt::vector x
blt::vector y
set subset *
label .l -image $dest 
$dest blank white
set bg [image create picture -file images/blt98.gif]
$dest copy $bg
pack .l
foreach region [array names us_regions $subset] {
    set coords $us_regions($region)
    set cnum [expr {int(rand()*14.0)}]
    all set $coords
#     all split x y
#     set min [blt::vector expr min(x)]
#     x expr { (x-$min)*3.0 + 10 }
#     set min [blt::vector expr min(y)]
#     y expr { (y-$min)*3.0 + 10 }
#     all merge x y 
    all expr { all * 2.0 }
    set coords [all print]
    $dest draw polygon -coords $coords -color $colors($cnum) \
	-antialiased yes -shadow 0 -alpha 155
    #$dest draw line -coords $coords -color black 
    #$dest draw rectangle 200 200 300 300 -color green -shadow 0 \
	-radius 10 -alpha 100
    $dest draw rectangle 200 200 400 300 -color blue -linewidth 29 \
	-radius 19 -shadow 0 -antialiased 0 -alpha 180
    incr count
}




set colors {
    aliceblue
    antiquewhite
    aqua
    aquamarine
    azure
    beige
    bisque
    black
    blanchedalmond
    blue
    blueviolet
    brown
    burlywood
    cadetblue
    chartreuse
    chocolate
    coral
    cornflowerblue
    cornsilk
    crimson
    cyan
    darkblue
    darkcyan
    darkgoldenrod
    darkgray
    darkgreen
    darkkhaki
    darkmagenta
    darkolivegreen
    darkorange
    darkorchid
    darkred
    darksalmon
    darkseagreen
    darkslateblue
    darkslategray
    darkturquoise
    darkviolet
    deeppink
    deepskyblue
    dimgray
    dodgerblue
    firebrick
    floralwhite
    forestgreen
    fuchsia
    gainsboro
    ghostwhite
    gold
    goldenrod
    gray
    green
    greenyellow
    honeydew
    hotpink
    indianred
    indigo
    ivory
    khaki
    lavender
    lavenderblush
    lawngreen
    lemonchiffon
    lightblue
    lightcoral
    lightcyan
    lightgoldenrodyellow
    lightgreen
    lightgrey
    lightpink
    lightsalmon
    lightseagreen
    lightskyblue
    lightslategray
    lightsteelblue
    lightyellow
    lime
    limegreen
    linen
    magenta
    #008000
    mediumaquamarine
    mediumblue
    mediumorchid
    mediumpurple
    mediumseagreen
    mediumslateblue
    mediumspringgreen
    mediumturquoise
    mediumvioletred
    midnightblue
    mintcream
    mistyrose
    moccasin
    navajowhite
    navy
    oldlace
    olive
    olivedrab
    orange
    orangered
    orchid
    palegoldenrod
    palegreen
    paleturquoise
    palevioletred
    papayawhip
    peachpuff
    peru
    pink
    plum
    powderblue
    purple
    red
    rosybrown
    royalblue
    saddlebrown
    salmon
    sandybrown
    seagreen
    seashell
    sienna
    silver
    skyblue
    slateblue
    slategray
    snow
    springgreen
    steelblue
    tan
    teal
    thistle
    tomato
    turquoise
    violet
    wheat
    white
    whitesmoke
    yellow
    yellowgreen
}

set imgData {
    R0lGODlhEAANAMIAAAAAAH9/f///////AL+/vwAA/wAAAAAAACH5BAEAAAUALAAAAAAQAA0A
    AAM8WBrM+rAEQWmIb5KxiWjNInCkV32AJHRlGQBgDA7vdN4vUa8tC78qlrCWmvRKsJTquHkp
    ZTKAsiCtWq0JADs=
}

set icon2 [image create picture -file images/blt98.gif]
set icon [image create picture -data $imgData]
set bg [blt::bgstyle create gradient -high  grey100 -low grey90 \
	-dir x -jitter yes -log yes -relativeto self]

set image ""
option add *ComboEntry.takeFocus 1

if { [file exists ../library] } {
    set blt_library ../library
}

set myIcon ""
blt::comboentry .e \
    -image $image \
    -textvariable myText1 \
    -iconvariable myIcon1 \
    -textwidth 6 \
    -menu .e.m \
    -menuanchor se \
    -exportselection yes \
    -xscrollcommand { .s set }  \
    -postcommand {.e.m configure -width [winfo width .e] ; update} \
    -command "puts {button pressed}"


set labels { 
    Aarhus Aaron Ababa aback abaft abandon abandoned abandoning
    abandonment abandons abase abased abasement abasements abases
    abash abashed abashes abashing abasing abate abated abatement
    abatements abater abates abating Abba abbe abbey abbeys abbot
    abbots Abbott abbreviate abbreviated abbreviates abbreviating
    abbreviation abbreviations Abby abdomen abdomens abdominal
    abduct abducted abduction abductions abductor abductors abducts
    Abe abed Abel Abelian Abelson Aberdeen Abernathy aberrant
    aberration aberrations abet abets abetted abetter abetting
    abeyance abhor abhorred abhorrent abhorrer abhorring abhors
    abide abided abides abiding Abidjan Abigail Abilene abilities
    ability abject abjection abjections abjectly abjectness abjure
    abjured abjures abjuring ablate ablated ablates ablating
    ablation ablative ablaze able abler ablest ably Abner abnormal
    abnormalities abnormality abnormally Abo aboard abode abodes
    abolish abolished abolisher abolishers abolishes abolishing
    abolishment abolishments abolition abolitionist abolitionists
    abominable abominate aboriginal aborigine aborigines abort
    aborted aborting abortion abortions abortive abortively aborts
    Abos abound abounded abounding abounds about above aboveboard
    aboveground abovementioned abrade abraded abrades abrading
    Abraham Abram Abrams Abramson abrasion abrasions abrasive
    abreaction abreactions abreast abridge abridged abridges
    abridging abridgment abroad abrogate abrogated abrogates
    abrogating abrupt abruptly abruptness abscess abscessed
    abscesses abscissa abscissas abscond absconded absconding
    absconds absence absences absent absented absentee
    absenteeism absentees absentia absenting absently absentminded
    absents absinthe absolute absolutely absoluteness absolutes
    absolution absolve absolved absolves absolving absorb
    absorbed absorbency absorbent absorber absorbing absorbs
    absorption absorptions absorptive abstain abstained abstainer
    abstaining abstains abstention abstentions abstinence
    abstract abstracted abstracting abstraction abstractionism
    abstractionist abstractions abstractly abstractness
    abstractor abstractors abstracts abstruse abstruseness
    absurd absurdities absurdity absurdly Abu abundance abundant
    abundantly abuse abused abuses abusing abusive abut abutment
    abuts abutted abutter abutters abutting abysmal abysmally
    abyss abysses Abyssinia Abyssinian Abyssinians acacia
    academia academic academically academics academies academy
    Acadia Acapulco accede acceded accedes accelerate accelerated
    accelerates accelerating acceleration accelerations
    accelerator accelerators accelerometer accelerometers accent
    accented accenting accents accentual accentuate accentuated
    accentuates accentuating accentuation accept acceptability
    acceptable acceptably acceptance acceptances accepted
    accepter accepters accepting acceptor acceptors accepts
    access accessed accesses accessibility accessible accessibly
    accessing accession accessions accessories accessors
    accessory accident accidental accidentally accidently
    accidents acclaim acclaimed acclaiming acclaims acclamation
    acclimate acclimated acclimates acclimating acclimatization
    acclimatized accolade accolades accommodate accommodated
    accommodates accommodating accommodation accommodations
    accompanied accompanies accompaniment accompaniments
    accompanist accompanists accompany accompanying accomplice
    accomplices accomplish accomplished accomplisher accomplishers
    accomplishes accomplishing accomplishment accomplishments
    accord accordance accorded accorder accorders according
    accordingly accordion accordions accords accost accosted
    accosting accosts account accountability accountable accountably
    accountancy accountant accountants accounted accounting
    accounts Accra accredit accreditation accreditations
    accredited accretion accretions accrue accrued accrues
    accruing acculturate acculturated acculturates acculturating
    acculturation accumulate accumulated accumulates accumulating
    accumulation accumulations accumulator accumulators
    accuracies accuracy accurate accurately accurateness accursed
    accusal accusation accusations accusative accuse accused
    accuser accuses accusing accusingly accustom accustomed
    accustoming accustoms ace aces acetate acetone acetylene
    Achaean Achaeans ache ached aches achievable achieve achieved
    achievement achievements achiever achievers achieves achieving
    Achilles aching acid acidic acidities acidity acidly acids
    acidulous Ackerman Ackley acknowledge acknowledgeable
    acknowledged acknowledgement acknowledgements acknowledger
    acknowledgers acknowledges acknowledging acknowledgment
    acknowledgments acme acne acolyte acolytes acorn acorns
    acoustic acoustical acoustically acoustician acoustics
    acquaint acquaintance acquaintances acquainted acquainting
    acquaints acquiesce acquiesced acquiescence acquiescent
    acquiesces acquiescing acquirable acquire acquired acquires
    acquiring acquisition acquisitions
}

blt::combomenu .e.m \
    -textvariable myText1 \
    -iconvariable myIcon1 \
    -disabledforeground grey45  \
    -disabledbackground grey85  \
    -disabledacceleratorforeground grey45  \
    -width -400 \
    -height -500 \
    -xscrollcommand { .e.m.xbar set } \
    -yscrollcommand { .e.m.ybar set } \
    -yscrollbar .e.m.ybar \
    -xscrollbar .e.m.xbar

.e.m listadd $labels \
    -icon $icon 

blt::tile::scrollbar .e.m.xbar  \
    -orient horizontal \
    -command { .e.m xview } \
    -width 17

blt::tile::scrollbar .e.m.ybar \
    -orient vertical \
    -command { .e.m yview } \
    -highlightthickness 0 -width 17 

blt::tile::scrollbar .s -orient vertical -command { .e xview } 

bind ComboEntry <3> {
    grab release [grab current]
}

blt::table . \
    0,0 .e -fill both -cspan 2 -padx 2 -pady 2 \

blt::table configure . c1 -resize shrink
blt::table configure . r0 -resize shrink
blt::table configure . c0 -pad { 2 0 }
blt::table configure . c1 -pad { 0 2 }

puts [.e configure -text]
#blt::bltdebug 100
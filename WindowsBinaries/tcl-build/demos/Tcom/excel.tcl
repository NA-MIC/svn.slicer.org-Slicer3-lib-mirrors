# $Id: excel.tcl,v 1.9 2001/06/30 18:42:58 cthuang Exp $
#
# This example controls Excel.  It performs the following steps.
#       - Start Excel application.
#       - Create a new workbook.
#       - Put values into some cells.
#       - Save the workbook to a file.
#       - Exit Excel application.

package require tcom

# Print the properties and methods exposed by the object.

proc dumpInterface {obj} {
    set interface [::tcom::info interface $obj]

    set properties [$interface properties]
    foreach property $properties {
        puts "property $property"
    }

    set methods [$interface methods]
    foreach method $methods {
	puts "method [lrange $method 0 2] \{"
	set parameters [lindex $method 3]
	foreach parameter $parameters {
            puts "    \{$parameter\}"
	}
	puts "\}"
    }
}

set application [::tcom::ref createobject "Excel.Application"]
$application Visible 1

set workbooks [$application Workbooks]
set workbook [$workbooks Add]
set worksheets [$workbook Worksheets]
set worksheet [$worksheets Item [expr 1]]

set cells [$worksheet Cells]
set i 0
foreach row {1 2 3} {
    foreach column {A B C} {
        $cells Item $row $column [incr i]
    }
}

$workbook SaveAs {c:\tst.xls}
$application Quit

# $Id: chart.tcl,v 1.1 2001/08/18 00:35:52 cthuang Exp $
#
# This example controls Excel.  It performs the following steps.
#       - Start Excel application.
#       - Create a new workbook.
#       - Put values into some cells.
#       - Create a chart.

package require tcom

set application [::tcom::ref createobject "Excel.Application"]
$application Visible 1

set workbooks [$application Workbooks]
set workbook [$workbooks Add]
set worksheets [$workbook Worksheets]
set worksheet [$worksheets Item [expr 1]]

set cells [$worksheet Cells]
$cells Item 1 A "North"
$cells Item 1 B "South"
$cells Item 1 C "East"
$cells Item 1 D "West"
$cells Item 2 A 5.2
$cells Item 2 B 10.0
$cells Item 2 C 8.0
$cells Item 2 D 20.0
set sourceRange [$worksheet Range "A1" "D2"]

set charts [$workbook Charts]
set chart [$charts Add]
$chart ChartWizard \
    $sourceRange \
    [expr -4102] \
    [expr 7] \
    [expr 1] \
    [expr 1] \
    [expr 0] \
    0 \
    "Sales Percentages"

# Prevent Excel from prompting to save the document on close.
$workbook Saved 1

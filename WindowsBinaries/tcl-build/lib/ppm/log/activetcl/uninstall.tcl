# -*- tcl -*-
# Copyright (C) 2001-2004 ActiveState Corporation May 05, 2004
#
# Information for the installer which is dependent on the chosen
# distribution. Original source of the information is the script
# "build/setvars.sh".
#
# ActiveState ActiveTcl 8.4.6.1 Win32/x86, Wed May 05 1:34:31 PM Pacific Daylight Time 2004
#

global    AT
array set AT {
    build       {99631}
    NAME	{ActiveTcl}
    PGROUP	{ActiveTcl}
    dPGROUP	{ActiveTcl}
    VERSION	8.4.6.1
    MATURITY	final
    Company	ActiveState
    url,about	http://www.ActiveState.com/Products/ActiveTcl/
    url,update	http://aspn.activestate.com/ASPN/Downloads/ActiveTcl/
    url,help	http://www.ActiveState.com/
    MODE	normal
    HELP	{ActiveTclHelp.chm}
    UNINSTALLER uninstall.tcl
    BLD_PFXPATH  C:/cygwin/home/Administrator/dbn/lba/night/builds/win32-ix86/out
    BLD_EPFXPATH C:/cygwin/home/Administrator/dbn/lba/night/builds/win32-ix86/out/win32-ix86
    DEBUG        0
    tcl_VERSION  8.4
    ARCH         win32-ix86
    TDK_ATVERS   8.4.6.1
}

set AT(LOG) [file join \
	lib ppm log [string tolower $AT(PGROUP)] install.log]

# When this file is used by the uninstaller SCRIPT_DIR is not defined.

if {[info exists ::SCRIPT_DIR]} {
    set _welcome [file join $::SCRIPT_DIR install_welcome.txt]

    global WELCOME
    set    WELCOME [read [set fh [open $_welcome r]]][close $fh]
    unset _welcome
}

if {[string equal $::AT(MODE) direct]} {
    # For the purposes of the installation the 'direct' distribution
    # is the same the 'normal' ActiveTcl one.
    set ::AT(MODE) normal
}

# ==================================================================

# -*- tcl -*-
# Uninstallation script for ActiveTcl
# ----------------------------------------------
#
# Copyright 2001, ActiveState Corp.
# All Rights Reserved.

##
### Assumption: The contents of install_data.tcl were prepended to this script.
##

#
# This should be run using the ActiveTcl wish version that we will be
# uninstalling
#

# This can be used with 8.3.3.3+

package require ActiveTcl 8.3.3.3
package require Tk
package require BWidget

option add *Button.highlightthickness 1
option add *Scrollbar.highlightthickness 1
option add *Text.highlightthickness 1
option add *Label.highlightthickness 0
option add *Label.borderWidth 0

set ::WELCOME "Uninstaller for $::AT(NAME) $::AT(VERSION)

Select 'Next' to begin uninstalling.
"

# ----------------------------------------------

proc main {} {
    wm protocol . WM_DELETE_WINDOW exit
    wm title . "$::AT(Company) $::AT(NAME) $::AT(VERSION) Uninstaller"

    # Read the logo and create an image from it.

    set logo [image create photo -file \
	    [file join $::tk_library images ActiveTclSplash.gif]]

    set left [frame .left]
    set ::BASE [PagesManager .right]
    set sep  [frame .sep -height 2 -bd 2 -relief sunken]
    set ::BTNS [frame .btns]

    label $left.logo -image $logo
    pack  $left.logo -expand 1 -fill both

    grid $left $::BASE -sticky news
    grid $sep -columnspan 2 -sticky ew
    grid $::BTNS -columnspan 2 -sticky ew
    grid columnconfig $::BTNS 0 -weight 1
    grid columnconfig . 1 -weight 1
    grid rowconfigure . 0 -weight 1

    set pady [expr {($::tcl_version > 8.3) ? 2 : 0}]
    set ::CANCEL [button $::BTNS.cncl -text "Cancel" -width 8 -pady $pady \
		      -command { cancel }]
    set ::NEXT   [button $::BTNS.next -text "Next >" -width 8 -pady $pady \
		      -command {set ::WAIT 1}]

    grid $::NEXT $::CANCEL -sticky e -padx 6 -pady 8

    # Note: The procedures open and manipulate the user interface.
    # They use [vwait] to enter the eventloop where needed so that
    # sequencing control is not taken from [main].

    uninstall $::BASE

    exit 0
}

# ----------------------------------------------

proc uninstall {pages} {
    set pname uninstall
    set page [$pages getframe $pname]
    set firsttime 0
    if {![winfo exists $page]} {
	set firsttime 1
	set page [$pages add $pname]
	grid [ScrolledWindow $page.s -auto both] -sticky news -padx 4 -pady 4
	set tw [text $page.s.t -width 20 -height 8]
	$page.s setwidget $tw

	grid columnconfigure $page 0 -weight 1
	grid rowconfigure $page 0 -weight 1

	$tw insert 1.0 $::WELCOME
	$tw configure -state disabled
	$tw tag configure error -background #CC4444

	# Accept focus even when disabled
	bind $tw <1>		{ focus %W }
	bind $tw <Key-Return>	{ next }
    }
    $pages raise $pname
    set ::LOGWIN $page.s.t

    set res [wait_next]
    if {$res < 0} { return $res }

    # At this point, there is only cancel
    $::NEXT configure -state disabled

    # Uninstall stuff
    do_uninstall_modules

    # This only allows exit
    if {[string equal "windows" $::tcl_platform(platform)]} {
	wait_next 0 "Finish"
    } else {
	wait_next 0 "Exit"
    }
}

# ----------------------------------------------
# WAIT ROUTINES
# ----------------------------------------------

proc next {} { $::NEXT invoke }
proc cancel {} { exit 0 }

proc wait_next {{next 1} {cancel "Cancel"}} {
    if {$next} { grid $::NEXT } else { grid remove $::NEXT }
    $::NEXT configure -state normal -command [list set ::WAIT $next]
    $::CANCEL configure -text $cancel -state normal -command cancel

    vwait ::WAIT
    return $::WAIT
}

# ----------------------------------------------
# LOGGING ROUTINE
# ----------------------------------------------

proc log {msg {type ok}} {
    if {[string length $msg]} {
	$::LOGWIN configure -state normal
	$::LOGWIN insert end "$msg\n" $type
	$::LOGWIN see end
	$::LOGWIN configure -state disabled
	update
    }
}

proc registry_root {} {
    global INSTALL_MODE ; # "admin" or "user"
    switch -exact -- $INSTALL_MODE {
	admin   {return HKEY_LOCAL_MACHINE}
	user    {return HKEY_CURRENT_USER}
	default {
	    return -code error "PANIC.\
		    Internal error of the installer.\
		    This should not happen."
	}
    }
}

proc soft_root {}  {return "[registry_root]\\SOFTWARE"}

proc env_root {} {
    global INSTALL_MODE
    switch -exact -- $INSTALL_MODE {
	admin {
	    return {HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment}
	}
	user {
	    return {HKEY_CURRENT_USER\Environment}
	}
	default {
	    return -code error "PANIC.\
		    Internal error of the uninstaller.\
		    This should not happen."
	}
    }
}


proc do_uninstall_modules {} {
    global INSTALL_MODE; # "admin" or "user"

    log "\nUninstalling $::AT(NAME) $::AT(VERSION) ..."
    log "Reading in [file tail $::AT(LOG)] ..."

    set data ""

    if {[string equal $::AT(MODE) lite] || [string equal $::AT(MODE) pro]} {
	package require TclDevKit
	set ver $::activestate::TclDevKit(release)
    } else {
	set ver $::AT(VERSION)
    }

    set isCurrentVersion 0 ; # Default for Unix to skip registry entries.

    ## Bugzilla 19731 ... The log file is always a sibling to the
    # uninstaller. Always use this relationship to find the correct
    # log file, and _not_ the registry. Entries in the registry may
    # point to a different installation of ActiveTcl and/or
    # TclPro. IOW they are not reliable for finding the correct file.

    if {[catch {
	# LOG is sibling to the uninstall script.
	set fh [open [file join $::SCRIPT_DIR [file tail $::AT(LOG)]]]
	set data [read $fh]
	close $fh
    } err]} {
	log "ERROR: $err" error
    }
    set data [split $data \n]

    ## Determine the type of the installation. This in turn tell us
    ## where to find the registry entries dealing with this
    ## installation, i.e. HKEY_LOCAL_MACHINE vs. HKEY_CURRENT_USER
    ## on windows.

    set found 0
    foreach line $data {
	if {$line == "" || [string match "#*" $line]} { continue }
	if {[string equal [lindex $line 0] INSTALLTYPE]} {
	    set INSTALL_MODE [lindex $line 1]
	    set found 1
	    break
	}
	# ignore all other information in the log.
    }

    if {!$found} {
	# Unable to determine type of installation. Aborting.

	log "Unable to determine installation type.\nABORTING UNINSTALL."
	return
    }

    if {[string equal "windows" $::tcl_platform(platform)]} {
	package require registry
	package require dde

	# Look into the registry using the knowledge of common
	# vs. personal installation. We do not use this to find
	# the log file. This was already done.

	set key [soft_root]\\$::AT(Company)\\$::AT(PGROUP)

	if {[catch {registry get "$key\\$ver" ""} verdir]} {
	    log "Inconsistent uninstall state.\nABORTING UNINSTALL."
	    return
	}

	if {![catch {registry get $key "CurrentVersion"} curver] \
		&& [string equal $curver $ver]} {
	    set isCurrentVersion 1
	} else {
	    # We are not the current version. We remove the
	    # information for this version from the registry, but
	    # nothing else.

	    log "Deleting registry key $key\\$ver"
	    if {[catch {
		registry delete "$key\\$ver"
	    } err]} {
		log "ERROR: $err" error
	    }
	}

	# It is not necessary to delete CurrentVersion\\Uninstall\\$::AT(PGROUP)
	# here. This key is listed in the log file now if it was used at all.
	# IOW this is no special case anymore.

	if {[string equal "Windows NT" $::tcl_platform(os)] && \
		![string equal $::AT(MODE) expect4win]} {
	    set environ [env_root]

	    # The directory for the binaries of the installation is also found
	    # relative to the uninstaller, and not through the registry.

	    set bindir [file nativename \
		[file join [file dirname [file dirname [file dirname \
		[file dirname $::SCRIPT_DIR]]]] bin]]
	    #set bindir $verdir\\bin

	    log "Removing \"$bindir\" from your executable path ..."

	    # Path might not be set. Situation: Personal installation
	    # where the chosen path is already in the System PATH. In
	    # that case the user PATH is not extended/set. Hence the
	    # uninstall will not find anything to remove the path
	    # from.

	    if {![catch {
		set curPath [registry get $environ Path]
	    }]} {
		set newPath [string map -nocase [list "$bindir;" {}] $curPath]
		if {[catch {
		    registry set $environ Path $newPath \
			[registry type $environ Path]
		} err]} { log "ERROR: $err" error }
	    } else {
		log "WARNING: Unable to retrieve path environment variable" \
		    error
	    }
	}
	# We don't remove the .tcl in PATHEXT
    }

    # Now the general case, delete registry entries, collect files and
    # directories, sort them, write a script and run it to delete them.

    set dirs ""
    set files ""
    foreach line $data {
	if {$line == "" || [string match "#*" $line]} { continue }
	switch -exact [lindex $line 0] {
	    MKDIR	{
		set dir [lindex $line 1]
		if {[file isdirectory $dir]} { lappend dirs $dir }
	    }
	    FILECOPY	{
		set file [lindex $line 1]
		if {[file exists $file]} { lappend files $file }
	    }
	    REGKEY	{
		if {[llength $line] == 2} {
		    foreach {cmd skey} $line { break }
		    if {$isCurrentVersion} {
			log "Deleting registry key $skey"
			# We don't worry about errors here - we only
			# care that the reg key is gone.
			catch {registry delete $skey}
		    }
		} else {
		    foreach {cmd skey value data} $line { break }
		    if {$isCurrentVersion} {
			log "Deleting registry key $skey $value"
			# We don't worry about errors here - we only
			# care that the reg key is gone.
			catch {registry delete $skey $value}
		    }
		}
	    }
	    PROGGROUP	{
		set group [lindex $line 1]
		log "Deleting Program Group $group"
		if {[catch {
		    dde execute progman progman "\[DeleteGroup($group)\]"
		} err]} {
		    log "dde returned: $err"
		}
	    }
	    INSTALLTYPE {
		# Ignore here.
	    }
	    default {
		log "Unknown LOG instruction \"$line\""
	    }
	}
    }
    if {[string equal "windows" $::tcl_platform(platform)] && \
	    ![string equal $::AT(MODE) expect4win] && \
	    ([llength $files] || [llength $dirs])} {
	if {[info exists ::env(TEMP)]} {
	    set uninst [file join $::env(TEMP) at-unins.bat]
	} elseif {[info exists ::env(TMP)]} {
	    set uninst [file join $::env(TMP) at-unins.bat]
	} else {
	    set uninst "C:/at-unins.bat"
	}
	if {[catch {open $uninst w} fid]} {
	    log "ERROR: Unable to create bat file to remove files" error
	} else {
	    # A little hacky, but on Windows, since we can't delete
	    # Ourselves, we wait on the "Finish" to launch a batch file
	    # which does the file and directory removal
	    rename exit real_exit
	    set cmd [concat [auto_execok start] " \"$uninst\""]
	    proc exit args "exec $cmd & ; uplevel 1 real_exit \$args"
	    puts $fid "@ECHO OFF"
	    puts $fid "REM This script removes the installation files"
	    puts $fid "REM for $::AT(NAME) $ver.  It can be deleted after run.\n"
	    puts $fid "REM Make sure we are not in the install directory."
	    puts $fid "CD C:\\"

	    # try and get the system 'find', since other exes might
	    # not be compatible (ie: cygwin).
	    set find "find.exe"
	    set sysdirs {
		c:/windows/system32
		c:/winnt/system32
		c:/windows/command
	    }
	    foreach dir $sysdirs {
		if {[file exists [file join $dir $find]]} {
		    set find [file nativename [file join $dir $find]]
		    break
		}
	    }

	    if {[string equal "Windows NT" $::tcl_platform(os)]} {
		# NT/2K batch scripting is a bit more enhanced
		# All the extras aren't really necessary, but my purist
		# scripting heart likes this better.
		puts $fid "ECHO Press Return to remove all files installed\
			with $::AT(NAME) $ver ..."
		puts $fid "PAUSE > NUL"

		# Files removed in a for loop
		puts $fid "FOR %%Z IN ("
		foreach file [lsort -unique -decreasing $files] {
		    set file [file nativename \
			    [file attributes $file -shortname]]
		    puts $fid "    $file"
		}
		puts $fid ") DO ("
		puts $fid "    IF EXIST %%Z ("
		puts $fid "        ECHO Removing file \"%%Z\""
		puts $fid "        DEL /F \"%%Z\""
		puts $fid "    )"
		puts $fid ")"

		# Dirs removed in a for loop
		puts $fid "REM Check to see that the directory is empty first"
		puts $fid "FOR %%Z IN ("
		# The lsort -decr sorts them in depth-first order
		foreach dir [lsort -unique -decreasing $dirs] {
		    set dir [file nativename [file attributes $dir -shortname]]
		    puts $fid "    $dir"
		}
		puts $fid ") DO ("
		puts $fid "    IF EXIST %%Z\\nul ("
		puts $fid "        DIR %%Z | $find \" 0 bytes\" > NUL"
		puts $fid "        IF NOT ERRORLEVEL==1 ("
		puts $fid "            ECHO Removing directory \"%%Z\""
		puts $fid "            RMDIR %%Z"
		puts $fid "        )"
		puts $fid "    )"
		puts $fid ")"

		# All done now
		puts $fid "ECHO Done.  Press Return to exit ..."
		puts $fid "PAUSE > NUL"
	    } else {
		# Due to bug 232731, we can only delay on Win9*, not PAUSE
		puts $fid "ECHO Removing all files installed\
			with $::AT(NAME) $ver ..."
		puts $fid "TYPE NUL | CHOICE.COM /N /CY /TY,3 >NUL"

		# This is more brute force, but Win9* batch scripting stinks.
		foreach file [lsort -unique -decreasing $files] {
		    set file [file nativename \
			    [file attributes $file -shortname]]
		    puts $fid "ECHO Removing \"$file\""
		    puts $fid "IF EXIST $file DEL $file"
		}

		# The lsort -decr sorts them in depth-first order
		foreach dir [lsort -unique -decreasing $dirs] {
		    set dir [file nativename [file attributes $dir -shortname]]
		    puts $fid "ECHO Removing \"$dir\""
		    puts $fid "IF EXIST $dir\\nul\
			    DIR $dir | $find \" 0 bytes\" > NUL"
		    puts $fid "IF NOT ERRORLEVEL==1 RMDIR $dir"
		}

		# All done now
		puts $fid "ECHO Done.  Exiting ..."
		puts $fid "TYPE NUL | CHOICE.COM /N /CY /TY,2 >NUL"
	    }

	    puts $fid "EXIT"
	    close $fid
	}

	# broadcast to other programs that we have changed the environment
	catch {registry broadcast Environment}

	log ""
	log "All other Tcl/Tk applications must be closed before finishing ..."
	log "Press Finish to uninstall associated files ..."
    } else {
	foreach file $files {
	    log "Removing file $file"
	    if {[catch {file delete -force $file} err]} {
		log "ERROR: $err" error
	    }
	}

	# The lsort -decr sorts them in depth-first order
	foreach dir [lsort -unique -decreasing $dirs] {
	    if {[llength [glob -nocomplain -dir $dir *]] == 0} {
		log "Removing empty directory $dir"
		if {[catch {file delete -force $dir} err]} {
		    log "ERROR: $err" error
		}
	    } else {
		log "Leaving non-empty directory $dir"
	    }
	}

	log "Done ..."
    }

    return
}

# ----------------------------------------------
# ----------------------------------------------

set ::SCRIPT_DIR [file dirname [info script]]
set here [pwd] ; cd $::SCRIPT_DIR ; set ::SCRIPT_DIR [pwd] ; cd $here

if {[catch {
    main
} err]} {
    catch {bgerror $err}
}
exit

# $Id: server.tcl,v 1.3 2002/06/29 15:34:52 cthuang Exp $
package provide Banking 1.0

package require tcom
::tcom::import [file join [file dirname [info script]] Banking.tlb]

proc accountImpl {method args} {
    global balance

    switch -- $method {
	_get_Balance {
	    return $balance
	}

	Deposit {
	    set amount [lindex $args 0]
	    set balance [expr $balance + $amount]
	}

	Withdraw {
	    set amount [lindex $args 0]
	    set balance [expr $balance - $amount]
	}
	
	default {
	    error "unknown method $method $args"
	}
    }
}

proc bankImpl {method args} {
    global balance

    switch -- $method {
	CreateAccount {
	    set balance 0
	    set name ""
	    return [::tcom::object create ::Banking::Account accountImpl]
	}
	
	default {
	    error "unknown method $method $args"
	}
    }
}

::tcom::object registerfactory ::Banking::Bank {list bankImpl}

package require tcom

set bank [::tcom::ref createobject "Banking.Bank"]
set account [$bank create]
puts [$account balance]
$account deposit 20
puts [$account balance]
$account withdraw 10
puts [$account balance]

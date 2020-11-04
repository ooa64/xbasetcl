if {[info exists ::tcl_platform(debug)]} {
    package ifneeded xbasetcl 1.0 [list load [file join $dir xbasetcl10g[info sharedlibextension]]]
} else {
    package ifneeded xbasetcl 1.0 [list load [file join $dir xbasetcl10[info sharedlibextension]]]
}


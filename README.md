# xbasetcl

Tcl interface to the XBASE library (http://linux.techass.com/projects/xdb).

xbasetcl 1.0 alpha 4

Tested: 

    Xbase: 2.1.3 from https://github.com/ooa64/xbase.
    Os/compiler: Windows/msvc, Linux/gcc.
    Tcl: 8.6.

Todo:

    - memo fields fix, field type checks.
    - expressions: xbase,dbf expr <expr>.
    - iterators: dbf,index,filter eval <expr_list> ?<var_list>? ?<script>?].
    - autoconf, TEA.
    - more tests.
    - more examlpes.
    - good documentation.

History:

    1.0a4 2020/11/07 - revived using xbase-2.1.3.
    1.0a3 2002/11/03 - flush command, fixes, samples.
    1.0a2 2002/09/03 - encoding, indexes, filters.
    1.0a1 2002/06/02 - basic dbf functionality.

Oleg Oleinick, ooa64@ua.fm

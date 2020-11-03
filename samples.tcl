catch {load ./xbasetcl1.0.so}
catch {load ./xbasetcl10.dll}

# sample1

set MyRecord {
	{"FIRSTNAME" C 15  0}
	{"LASTNAME"  C 20  0}
	{"BIRTHDATE" D  8  0}
	{"AMOUNT"    N  9  2}
	{"SWITCH"    L  1  0}
	{"FLOAT1"    F  9  2}
	{"FLOAT2"    F  9  1}
	{"FLOAT3"    F  9  2}
	{"FLOAT4"    F  9  3}
	{"MEMO1"     M 10  0}
	{"ZIPCODE"   N  5  0}      
};

[xbase  dbf MyFile]     create MYFILE.DBF   $MyRecord            -overlay
[MyFile index MyIndex1] create MYINDEX1.NDX "LASTNAME"           -overlay
[MyFile index MyIndex2] create MYINDEX2.NDX "LASTNAME+FIRSTNAME" -overlay
[MyFile index MyIndex3] create MYINDEX3.NDX "LASTNAME-FIRSTNAME" -overlay
[MyFile index MyIndex4] create MYINDEX4.NDX "ZIPCODE"            -overlay

MyFile append [list \
	Queue \
	Suzy \
	[clock format [clock seconds] -format %Y%m%d] \
	99.99 \
	Y \
	1.466 \
	1.466 \
	1 \
	1 \
	{} \
	76262]
MyFile append [list \
	Bob \
	Billy \
	19970304 \
	88.88 \
	N \
	-2.1 \
	-2.1 \
	-2.1 \
	-2.1 \
	{} \
	76261]
MyFile append [list \
	Slippery \
	Sam \
	19970406 \
	77.77 \
	T \
	3.21 \
	3.21 \
	3.21 \
	3.21 \
	{} \
	7626]
MyFile append [list \
	Lucas \
	George \
	19470406 \
	77.77 \
	T \
	4.321 \
	4.321 \
	4.321 \
	4.321 \
	{} \
	76260]

# exfilter

MyFile filter f1 "FLOAT1>1"

set rc [f1 first]
while {$rc} {
    puts "Filter 1 Found Record [MyFile pos]"
    set rc [f1 next]
}

MyIndex1 filter f2 "FLOAT1>1"

set rc [f2 last]
while {$rc} {
    puts "Filter 2 Found Record [MyFile pos]"
    set rc [f2 prev]
}


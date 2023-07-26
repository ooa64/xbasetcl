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

xbase dbf MyFile
MyFile version 4
MyFile create MYFILE.DBF $MyRecord -overlay

[MyFile index MyIndex1] create MYINDEX1.NDX "LASTNAME"           -overlay
[MyFile index MyIndex2] create MYINDEX2.NDX "LASTNAME+FIRSTNAME" -overlay
[MyFile index MyIndex3] create MYINDEX3.NDX "LASTNAME-FIRSTNAME" -overlay
[MyFile index MyIndex4] create MYINDEX4.NDX "ZIPCODE"            -overlay

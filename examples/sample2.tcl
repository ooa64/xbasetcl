catch {load ./xbasetcl1.0.so}
catch {load ./xbasetcl10.dll}

# sample2

[xbase dbf MyFile] open MYFILE.DBF

[MyFile index MyIndex1] open MYINDEX1.NDX
[MyFile index MyIndex2] open MYINDEX2.NDX
[MyFile index MyIndex3] open MYINDEX3.NDX
[MyFile index MyIndex4] open MYINDEX4.NDX

puts "Last Name Id  = [MyFile schema -fieldno LASTNAME]"
puts "First Name Id = [MyFile schema -fieldno FIRSTNAME]"
puts "Birthdate Id  = [MyFile schema -fieldno BIRTHDATE]"
puts "Amount Id     = [MyFile schema -fieldno AMOUNT]"
puts "Switch Id     = [MyFile schema -fieldno SWITCH]"
puts "Float 1 Id    = [MyFile schema -fieldno FLOAT1]"
puts "Float 2 Id    = [MyFile schema -fieldno FLOAT2]"
puts "Float 3 Id    = [MyFile schema -fieldno FLOAT3]"
puts "Float 4 Id    = [MyFile schema -fieldno FLOAT4]"
puts "Memo1 Id      = [MyFile schema -fieldno MEMO1]"
puts "Zipcode Id    = [MyFile schema -fieldno ZIPCODE]"

MyFile record [list \
	Suzy \
	Queue \
	[clock format [clock seconds] -format %Y%m%d] \
	99.99 \
	Y \
	1.466 \
	1.466 \
	1 \
	1 \
	{} \
	76262]
puts [MyFile status]
MyFile append
puts [MyFile status]
MyFile append
MyFile append
exit
MyFile record [list \
	Billy \
	Bob \
	19970304 \
	88.88 \
	N \
	-2.1 \
	-2.1 \
	-2.1 \
	-2.1 \
	"Sample memo field 2" \
	76261]
MyFile append 

MyFile close; exit
MyFile append
MyFile blank
puts [MyFile record]
MyFile record [list \
	Sam \
	Slippery \
	19970406 \
	77.77 \
	T \
	3.21 \
	3.21 \
	3.21 \
	3.21 \
	{} \
	7626]
puts [MyFile record]
MyFile append
MyFile blank
MyFile record [list \
	George \
	Lucas \
	19470406 \
	77.77 \
	T \
	4.321 \
	4.321 \
	4.321 \
	4.321 \
	{"Sample memo field 4"} \
	76260]
puts [MyFile record]
MyFile append


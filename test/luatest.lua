function main(p1)
	print("Hello!");
	print("test2()="..test2(p1));
	print("test3()="..test3(p1));
	t = TEST(p1);
--	t = TESTVEC(p2);
	print '------------';
	for k,v in pairs(t) do
		print(k.." -> "..v)
	end
	print '------------';
end
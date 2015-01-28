.load libferrissqlitevtable.so
create virtual table fs using libferris(
	'google://spreadsheets/smalltest1/Sheet1',
	'',
	name text,
        a text,
        b int,
        c int,
        d int,
        e text,
        f text,
        size int
        );

.headers on
-- select a,b,c,d,b+c as result
--     from fs
--    where a = 'foo';

-- update fs set f = 'libferris' where a = 'foo';


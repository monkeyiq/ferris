.headers on
.load libferrissqlitevtable.so
create virtual table fs using libferris(
	'google://spreadsheets/smalltest1/grouping',
	'',
	name text,
        a text,
        b text,
        c text,
        d int,
        e text,
        f text,
        size int
        );


select max(b),max(c),avg(d) as mean, sum(d) as total 
    from fs
   where not (b = '' or c = '')
   group by b,c;


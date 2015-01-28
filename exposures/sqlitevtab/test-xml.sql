.load libferrissqlitevtable.so
-- drop table fs;
create virtual table fs using libferris(
	'customers.xml/customers',
	'recursive=tru,',
	name text,
	size int,
	id int,
	givenname text,
	familyname text
	content text 
	);

select * from fs 
order by givenname desc;

-- ./sqlite3 -init test-xml.sql
-- file:///ferris/exposures/sqlitevtab/customers.xml/customers/customer--2|customer--2|0|131|Ziggy|Stardust
-- file:///ferris/exposures/sqlitevtab/customers.xml/customers/customer|customer|0|3|Foo|Bar
-- file:///ferris/exposures/sqlitevtab/customers.xml/customers/customer--1|customer--1|0|15|Bobby|McGee


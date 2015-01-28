.load libferrissqlitevtable.so
-- drop table fs;
create virtual table fs using libferris(
	'file:///tmp/customers.xml/customers',
	'',
	name text,
	size int,
	id int,
	givenname text,
	familyname text
	content text 
	);

insert into fs values ( 'file:///tmp/customers.xml/customers/customer',
	'customer',
	0,
	'new given name here2',
	'new sirname here',
	'' );

-- /bin/cp -f customers.xml /tmp
-- ./sqlite3 -init test-xml-update.sql
-- this will clobber the first customer record.

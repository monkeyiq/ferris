.load libferrissqlitevtable.so
-- drop table fs;
create virtual table fs using libferris(
	'/tmp/testrowids',
	'add-rowid-to-base-filesystem=true,',
	is-dir int,
	content text );

select url,content from fs 
order by url desc;

insert into fs values ( 'file:///tmp/testrowids/new1.txt', 0, 'content1' );
update fs set content = 'content2' where url = 'file:///tmp/testrowids/new1.txt';

select url,content from fs 
order by url desc;


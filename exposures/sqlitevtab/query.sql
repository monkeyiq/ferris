.load libferrissqlitevtable.so
drop table fs;
create virtual table fs using libferris(
	'eaq://(url=~Akashi-Kaikyo-Bridge)',
	'',
	name text,
	size int,
	md5 text, 
	path text,
	mtime int,
	atime int,
	ctime int,
	is-dir int,
	mtime-display text,
	width int,
	height int,
	content text );

select is_dir,width,height,name,url from fs 
order by url desc;



.load libferrissqlitevtable.so
-- drop table fs;
create virtual table fs using libferris(
	'/tmp/test',
	'recursive=tru,',
	name text,
	size int,
	md5 text, 
	path text,
	mtime int,
	atime int,
	ctime int,
	is-dir int,
	mtime-display text,
	content text );

select is_dir,name,url from fs 
order by url desc;

-- select md5,size,mtime,mtime_display,name,url from fs 
-- where  path = '/tmp/test/df1'
-- order by url desc;

-- .schema fs;


-- insert into fs values ( '/tmp/test/url','name',88,'md5','path',0,1,2,0,'mtime-display','content is here' );


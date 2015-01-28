.load libferrissqlitevtable.so
create virtual table fs using libferris(
	'amarok://playlist',
	'',
	name text,
	title text,
	artist text,
	album text,
	size int,
	length int, 
	playtime text,
        link-target text,
        ferris-delegate-url text					
        );

select name, playtime, title from fs 
order by name desc;

-- select avg(length) from fs;


select avg(length),max(artist),max(album)
from fs
group by artist, album;


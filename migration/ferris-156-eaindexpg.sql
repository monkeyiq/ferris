CREATE OR REPLACE FUNCTION fnshiftstring( varchar ) RETURNS varchar IMMUTABLE AS $fnshiftstringstamp$
DECLARE 
 earlin alias for $1;
 a text[];
 exploded text[];
 c record;
 s varchar;
BEGIN
  a := regexp_split_to_array( earlin, E'[/[\\]()\\.\\s]+');
  for c in select * from regexp_split_to_table( earlin, E'[/[\\]()\\.\\s]+') as f LOOP
     s := c.f::varchar;
     exploded := exploded || ARRAY[s::text];
     FOR i IN 1..10 LOOP
       s := substr( s, 2 );
       IF char_length( s ) < 3 THEN
          i := 100;
       ELSE
          exploded := exploded || ARRAY[s::text];
       END IF;
     END LOOP;
  END LOOP;
  return exploded::varchar;
END;
$fnshiftstringstamp$ LANGUAGE plpgsql;

create index urlmapfnshiftstring on urlmap using gin(to_tsvector('simple',fnshiftstring(url)));

CREATE OR REPLACE FUNCTION urlmatch( varchar, varchar, boolean ) 
  RETURNS setof int 
  IMMUTABLE AS $urlmatch$
DECLARE
  qconst alias for $1;
  eaname alias for $2;
  caseSensitive alias for $3;
  q varchar;
  r record;
BEGIN
  q := qconst;
  if not caseSensitive and regexp_replace( q, '[a-zA-Z0-9]*', '') = '' then
    for r in select urlid from urlmap where to_tsvector('simple',fnshiftstring(url)) @@ to_tsquery('simple', q || ':*' ) loop
      return next r.urlid;
    end loop;
  else
    if eaname = 'name' then
       q := '.*/' || qconst || '[^/]*$';
    end if;
    if caseSensitive then
      for r in select urlid from urlmap where url ~  q loop
        return next r.urlid;
      end loop;
    else
      for r in select urlid from urlmap where url ~* q loop
        return next r.urlid;
      end loop;
    end if;
  end if;    
  return;
END;
$urlmatch$ LANGUAGE plpgsql;

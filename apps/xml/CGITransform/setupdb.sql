#
#
#
create database if not exists ferristest;
use ferristest;

drop table if exists actors;
create table actors
(
  fname varchar(100),
  lname varchar(100),
  addr  varchar(100) default 'unknown address',
  custid int(10) primary key auto_increment
) type=MyISAM;

insert into actors (fname,lname,addr) values ('fred', 'smith', '15 backers' );
insert into actors (fname,lname,addr) values ('harry', 'sack', '15 credability st' );
insert into actors (fname,lname,addr) values ('frodo', 'baggins', 'bree' ); 
insert into actors (fname,lname,addr) values ('underhill', 'mcgoo', 'bree' );
insert into actors (fname,lname,addr) values ('sam', 'x', 'mordor' );
insert into actors (fname,lname,addr) values ('ring wrath', 'stealthboy', 'bree' );
insert into actors (fname,lname,addr) values ('cold drake', 'cold flame', 'bree' );

drop table if exists items;
create table items
(
  description varchar(100),
  iid  int(10) primary key auto_increment
) type=MyISAM;

insert into items (description) values ('bread');
insert into items (description) values ('sting');
insert into items (description) values ('staff');
insert into items (description) values ('cape');
insert into items (description) values ('xslt');
insert into items (description) values ('black cape');
insert into items (description) values ('the one ring');
insert into items (description) values ('magic boots');

drop table if exists holdings;
create table holdings
(
  custid int(10) not null,
  iid int(10) not null
) type=MyISAM;

insert into holdings (custid,iid) values ('1','1');
insert into holdings (custid,iid) values ('2','1');
insert into holdings (custid,iid) values ('3','1');
insert into holdings (custid,iid) values ('4','4');
insert into holdings (custid,iid) values ('5','5');
insert into holdings (custid,iid) values ('6','1');
insert into holdings (custid,iid) values ('7','3');

insert into holdings (custid,iid) values ('6','6');
insert into holdings (custid,iid) values ('3','7');
insert into holdings (custid,iid) values ('3','8');

select fname, description
from holdings,items,actors
where holdings.iid    = items.iid
and   holdings.custid = actors.custid

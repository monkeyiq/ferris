create database if not exists ferristest;
use ferristest;

drop table if exists ferrisxsl;
create table ferrisxsl (
  pk int(10) primary key auto_increment,
  userid int(5)  not null default '5',
  username varchar(100),
  addr varchar(100) default 'unknown address'
) type=MyISAM;

insert into ferrisxsl (userid,username,addr) values (1, 'fred', '15 backers' );
insert into ferrisxsl (userid,username,addr) values (5, 'harry', '15 credability st' );
insert into ferrisxsl (userid,username,addr) values (5, 'frodo', 'bags end' );
insert into ferrisxsl (userid,username,addr) values (5, 'underhill', 'bree' );
insert into ferrisxsl (userid,username,addr) values (5, 'sam', 'mordor' );
insert into ferrisxsl (userid,username,addr) values (2, 'strider2', 'bree' );
insert into ferrisxsl (userid,username,addr) values (2, 'strider3', 'bree' );
insert into ferrisxsl (userid,username,addr) values (2, 'strider4', 'bree' );
insert into ferrisxsl (userid,username,addr) values (2, 'strider5', 'bree' );

select * from ferrisxsl;


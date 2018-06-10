/******************************************************************************
*******************************************************************************
*******************************************************************************

    unit test code
    Copyright (C) 2003 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: ut_ferrisdom.cpp,v 1.1 2006/12/10 04:52:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include <Ferris/Ferris.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/Iterator.hh>
#include <Ferris/FilteredContext.hh>

#include <popt.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iterator>

using namespace std;
using namespace Ferris;

const string PROGRAM_NAME = "ut_ferrisdom";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

int errors = 0;

fh_ostream& E()
{
    ++errors;
    static fh_ostream ret = Factory::fcerr();
    return ret;
}

void
assertcompare( const std::string& emsg,
               const std::string& expected,
               const std::string& actual )
{
    if( expected != actual )
        E() << emsg << endl
            << " expected:" << expected << ":" 
            << " actual:" << actual << ":" << endl;
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
string BaseDir = "/tmp";



string testdoc = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
"<testdoc>"
"<childA>hi there</childA>"
"<childB/>"
"<childC><nested1/></childC>"
"</testdoc>\n";

string testdoc_pritty = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
"<testdoc>\n"
"\n"
"  <childA>hi there</childA>\n"
"\n"
"  <childB/>\n"
"\n"
"  <childC>\n"
"    <nested1/>\n"
"  </childC>\n"
"\n"
"</testdoc>\n";

string disjoint1_as_xml = \
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n"
"<context taken-from-url=\"file:///tmp/testbase/disjoint1\">\n"
"\n"
"  <keyval key=\"aspect-ratio\"></keyval>\n"
"\n"
"  <keyval key=\"associated-branches\">branchfs-attributes,branchfs-medallions,branchfs-parents,branchfs-extents,branchfs-signatures</keyval>\n"
"\n"
"  <keyval key=\"associated-branches-url\">branches://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"atime\">1126859303</keyval>\n"
"\n"
"  <keyval key=\"atime-ctime\">Fri Sep 16 18:28:23 2005\n"
"</keyval>\n"
"\n"
"  <keyval key=\"atime-day-granularity\">1126792800</keyval>\n"
"\n"
"  <keyval key=\"atime-display\">05 Sep 16 18:28</keyval>\n"
"\n"
"  <keyval key=\"atime-month-granularity\">1125496800</keyval>\n"
"\n"
"  <keyval key=\"atime-year-granularity\">1104501600</keyval>\n"
"\n"
"  <keyval key=\"attribute-count\">582</keyval>\n"
"\n"
"  <keyval key=\"block-count\">0</keyval>\n"
"\n"
"  <keyval key=\"block-size\">4096</keyval>\n"
"\n"
"  <keyval key=\"branchfs-attributes\">branchfs-attributes://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"branchfs-extents\">branchfs-extents://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"branchfs-medallions\">branchfs-medallions://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"branchfs-parents\">branchfs-parents://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"branchfs-signatures\">branchfs-gpg-signatures://file/tmp/testbase/disjoint1\n"
"</keyval>\n"
"\n"
"  <keyval key=\"ctime\">1126845276</keyval>\n"
"\n"
"  <keyval key=\"ctime-ctime\">Fri Sep 16 14:34:36 2005\n"
"</keyval>\n"
"\n"
"  <keyval key=\"ctime-day-granularity\">1126792800</keyval>\n"
"\n"
"  <keyval key=\"ctime-display\">05 Sep 16 14:34</keyval>\n"
"\n"
"  <keyval key=\"ctime-month-granularity\">1125496800</keyval>\n"
"\n"
"  <keyval key=\"ctime-year-granularity\">1104501600</keyval>\n"
"\n"
"  <keyval key=\"deletable\">1</keyval>\n"
"\n"
"  <keyval key=\"depth\"></keyval>\n"
"\n"
"  <keyval key=\"depth-per-color\"></keyval>\n"
"\n"
"  <keyval key=\"device\">64781</keyval>\n"
"\n"
"  <keyval key=\"device-type\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-block-count\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-block-size\">4096</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-ctime\">1126845276</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-device\">64781</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-device-type\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-filesystem-filetype\">directory</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-group-owner-name\">ferristester</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-group-owner-number\">25000</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-hard-link-count\">2</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-has-holes\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-inode\">8481318</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-is-dir\">1</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-is-file\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-is-link\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-is-special\">0</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-size\">51</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-user-owner-name\">ferristester</keyval>\n"
"\n"
"  <keyval key=\"dontfollow-user-owner-number\">25000</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since\">0</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"download-if-mtime-since-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"ea-names\">as-rdf,as-text,as-xml,aspect-ratio,associated-branches,associated-branches-url,atime,atime-ctime,atime-day-granularity,atime-display,atime-month-granularity,atime-year-granularity,attribute-count,block-count,block-size,branchfs-attributes,branchfs-extents,branchfs-medallions,branchfs-parents,branchfs-signatures,content,ctime,ctime-ctime,ctime-day-granularity,ctime-display,ctime-month-granularity,ctime-year-granularity,deletable,depth,depth-per-color,device,device-type,dontfollow-block-count,dontfollow-block-size,dontfollow-ctime,dontfollow-device,dontfollow-device-type,dontfollow-filesystem-filetype,dontfollow-group-owner-name,dontfollow-group-owner-number,dontfollow-hard-link-count,dontfollow-has-holes,dontfollow-inode,dontfollow-is-dir,dontfollow-is-file,dontfollow-is-link,dontfollow-is-special,dontfollow-selinux-context,dontfollow-selinux-type,dontfollow-size,dontfollow-user-owner-name,dontfollow-user-owner-number,download-if-mtime-since,download-if-mtime-since-ctime,download-if-mtime-since-day-granularity,download-if-mtime-since-display,download-if-mtime-since-month-granularity,download-if-mtime-since-year-granularity,ea-names,ea-names-no-qual,emblem:generic-user-mtime,emblem:generic-user-mtime-ctime,emblem:generic-user-mtime-day-granularity,emblem:generic-user-mtime-display,emblem:generic-user-mtime-month-granularity,emblem:generic-user-mtime-year-granularity,emblem:has-fuzzy-generic-user,emblem:has-fuzzy-libferris,emblem:has-fuzzy-personalities,emblem:has-fuzzy-system,emblem:has-fuzzy-user,emblem:has-generic-user,emblem:has-libferris,emblem:has-personalities,emblem:has-system,emblem:has-user,emblem:libferris-mtime,emblem:libferris-mtime-ctime,emblem:libferris-mtime-day-granularity,emblem:libferris-mtime-display,emblem:libferris-mtime-month-granularity,emblem:libferris-mtime-year-granularity,emblem:list,emblem:list-ui,emblem:personalities-mtime,emblem:personalities-mtime-ctime,emblem:personalities-mtime-day-granularity,emblem:personalities-mtime-display,emblem:personalities-mtime-month-granularity,emblem:personalities-mtime-year-granularity,emblem:system-mtime,emblem:system-mtime-ctime,emblem:system-mtime-day-granularity,emblem:system-mtime-display,emblem:system-mtime-month-granularity,emblem:system-mtime-year-granularity,emblem:upset,emblem:user-mtime,emblem:user-mtime-ctime,emblem:user-mtime-day-granularity,emblem:user-mtime-display,emblem:user-mtime-month-granularity,emblem:user-mtime-year-granularity,exists-subdir,ferris-current-time,filesystem-filetype,force-passive-view,fs-available-block-count,fs-block-count,fs-block-size,fs-file-name-length-maximum,fs-file-nodes-free,fs-file-nodes-total,fs-free-block-count,fs-free-size,fs-id,fs-name,fs-type,gamma,google-maps-location,group-executable,group-owner-name,group-owner-number,group-readable,group-writable,hard-link-count,has-alpha,has-holes,has-subcontexts-guess,has-valid-signature,height,inode,is-active-view,is-animation-object,is-audio-object,is-dir,is-dir-try-automounting,is-file,is-image-object,is-link,is-native,is-remote,is-source-object,is-special,language-human,latitude,longitude,md2,md5,mdc2,mimetype,mimetype-from-content,mode,mtime,mtime-ctime,mtime-day-granularity,mtime-display,mtime-month-granularity,mtime-year-granularity,name,name-extension,other-executable,other-readable,other-writable,parent-name,path,preallocation-at-tail,protection-ls,protection-raw,readable,realpath,recommended-ea,recommended-ea-short,recommended-ea-union,recommended-ea-union-view,recursive-subcontext-count,recursive-subcontext-dir-count,recursive-subcontext-file-count,recursive-subcontext-hardlink-count,recursive-subcontext-max-depth,recursive-subcontext-oldest-atime,recursive-subcontext-oldest-atime-ctime,recursive-subcontext-oldest-atime-day-granularity,recursive-subcontext-oldest-atime-display,recursive-subcontext-oldest-atime-month-granularity,recursive-subcontext-oldest-atime-url,recursive-subcontext-oldest-atime-year-granularity,recursive-subcontext-oldest-ctime,recursive-subcontext-oldest-ctime-ctime,recursive-subcontext-oldest-ctime-day-granularity,recursive-subcontext-oldest-ctime-display,recursive-subcontext-oldest-ctime-month-granularity,recursive-subcontext-oldest-ctime-url,recursive-subcontext-oldest-ctime-year-granularity,recursive-subcontext-oldest-mtime,recursive-subcontext-oldest-mtime-ctime,recursive-subcontext-oldest-mtime-day-granularity,recursive-subcontext-oldest-mtime-display,recursive-subcontext-oldest-mtime-month-granularity,recursive-subcontext-oldest-mtime-url,recursive-subcontext-oldest-mtime-year-granularity,recursive-subcontext-size,recursive-subcontext-size-cdrom-count,recursive-subcontext-size-dvd-count,recursive-subcontext-size-human-readable,recursive-subcontext-size-in-blocks,recursive-subcontext-size-in-blocks-cdrom-count,recursive-subcontext-size-in-blocks-dvd-count,recursive-subcontext-size-in-blocks-human-readable,rgba-32bpp,rpm-buildtime,rpm-buildtime-ctime,rpm-buildtime-day-granularity,rpm-buildtime-display,rpm-buildtime-month-granularity,rpm-buildtime-year-granularity,rpm-distribution,rpm-group,rpm-info-url,rpm-is-config,rpm-is-doc,rpm-is-ghost,rpm-is-license,rpm-is-pubkey,rpm-is-readme,rpm-license,rpm-package,rpm-packager,rpm-release,rpm-summary,rpm-vendor,rpm-verify-device,rpm-verify-group,rpm-verify-md5,rpm-verify-mode,rpm-verify-mtime,rpm-verify-owner,rpm-verify-size,rpm-version,runable,schema:as-rdf,schema:as-text,schema:as-xml,schema:aspect-ratio,schema:associated-branches,schema:associated-branches-url,schema:atime,schema:atime-ctime,schema:atime-day-granularity,schema:atime-display,schema:atime-month-granularity,schema:atime-year-granularity,schema:attribute-count,schema:block-count,schema:block-size,schema:branchfs-attributes,schema:branchfs-extents,schema:branchfs-medallions,schema:branchfs-parents,schema:branchfs-signatures,schema:content,schema:ctime,schema:ctime-ctime,schema:ctime-day-granularity,schema:ctime-display,schema:ctime-month-granularity,schema:ctime-year-granularity,schema:deletable,schema:depth,schema:depth-per-color,schema:device,schema:device-type,schema:dontfollow-block-count,schema:dontfollow-block-size,schema:dontfollow-ctime,schema:dontfollow-device,schema:dontfollow-device-type,schema:dontfollow-filesystem-filetype,schema:dontfollow-group-owner-name,schema:dontfollow-group-owner-number,schema:dontfollow-hard-link-count,schema:dontfollow-has-holes,schema:dontfollow-inode,schema:dontfollow-is-dir,schema:dontfollow-is-file,schema:dontfollow-is-link,schema:dontfollow-is-special,schema:dontfollow-selinux-context,schema:dontfollow-selinux-type,schema:dontfollow-size,schema:dontfollow-user-owner-name,schema:dontfollow-user-owner-number,schema:download-if-mtime-since,schema:download-if-mtime-since-ctime,schema:download-if-mtime-since-day-granularity,schema:download-if-mtime-since-display,schema:download-if-mtime-since-month-granularity,schema:download-if-mtime-since-year-granularity,schema:ea-names,schema:ea-names-no-qual,schema:emblem:generic-user-mtime,schema:emblem:generic-user-mtime-ctime,schema:emblem:generic-user-mtime-day-granularity,schema:emblem:generic-user-mtime-display,schema:emblem:generic-user-mtime-month-granularity,schema:emblem:generic-user-mtime-year-granularity,schema:emblem:has-fuzzy-generic-user,schema:emblem:has-fuzzy-libferris,schema:emblem:has-fuzzy-personalities,schema:emblem:has-fuzzy-system,schema:emblem:has-fuzzy-user,schema:emblem:has-generic-user,schema:emblem:has-libferris,schema:emblem:has-personalities,schema:emblem:has-system,schema:emblem:has-user,schema:emblem:libferris-mtime,schema:emblem:libferris-mtime-ctime,schema:emblem:libferris-mtime-day-granularity,schema:emblem:libferris-mtime-display,schema:emblem:libferris-mtime-month-granularity,schema:emblem:libferris-mtime-year-granularity,schema:emblem:list,schema:emblem:list-ui,schema:emblem:personalities-mtime,schema:emblem:personalities-mtime-ctime,schema:emblem:personalities-mtime-day-granularity,schema:emblem:personalities-mtime-display,schema:emblem:personalities-mtime-month-granularity,schema:emblem:personalities-mtime-year-granularity,schema:emblem:system-mtime,schema:emblem:system-mtime-ctime,schema:emblem:system-mtime-day-granularity,schema:emblem:system-mtime-display,schema:emblem:system-mtime-month-granularity,schema:emblem:system-mtime-year-granularity,schema:emblem:upset,schema:emblem:user-mtime,schema:emblem:user-mtime-ctime,schema:emblem:user-mtime-day-granularity,schema:emblem:user-mtime-display,schema:emblem:user-mtime-month-granularity,schema:emblem:user-mtime-year-granularity,schema:exists-subdir,schema:ferris-current-time,schema:filesystem-filetype,schema:force-passive-view,schema:fs-available-block-count,schema:fs-block-count,schema:fs-block-size,schema:fs-file-name-length-maximum,schema:fs-file-nodes-free,schema:fs-file-nodes-total,schema:fs-free-block-count,schema:fs-free-size,schema:fs-id,schema:fs-name,schema:fs-type,schema:gamma,schema:google-maps-location,schema:group-executable,schema:group-owner-name,schema:group-owner-number,schema:group-readable,schema:group-writable,schema:hard-link-count,schema:has-alpha,schema:has-holes,schema:has-subcontexts-guess,schema:has-valid-signature,schema:height,schema:inode,schema:is-active-view,schema:is-animation-object,schema:is-audio-object,schema:is-dir,schema:is-dir-try-automounting,schema:is-file,schema:is-image-object,schema:is-link,schema:is-native,schema:is-remote,schema:is-source-object,schema:is-special,schema:language-human,schema:latitude,schema:longitude,schema:md2,schema:md5,schema:mdc2,schema:mimetype,schema:mimetype-from-content,schema:mode,schema:mtime,schema:mtime-ctime,schema:mtime-day-granularity,schema:mtime-display,schema:mtime-month-granularity,schema:mtime-year-granularity,schema:name,schema:name-extension,schema:other-executable,schema:other-readable,schema:other-writable,schema:parent-name,schema:path,schema:preallocation-at-tail,schema:protection-ls,schema:protection-raw,schema:readable,schema:realpath,schema:recommended-ea,schema:recommended-ea-short,schema:recommended-ea-union,schema:recommended-ea-union-view,schema:recursive-subcontext-count,schema:recursive-subcontext-dir-count,schema:recursive-subcontext-file-count,schema:recursive-subcontext-hardlink-count,schema:recursive-subcontext-max-depth,schema:recursive-subcontext-oldest-atime,schema:recursive-subcontext-oldest-atime-ctime,schema:recursive-subcontext-oldest-atime-day-granularity,schema:recursive-subcontext-oldest-atime-display,schema:recursive-subcontext-oldest-atime-month-granularity,schema:recursive-subcontext-oldest-atime-url,schema:recursive-subcontext-oldest-atime-year-granularity,schema:recursive-subcontext-oldest-ctime,schema:recursive-subcontext-oldest-ctime-ctime,schema:recursive-subcontext-oldest-ctime-day-granularity,schema:recursive-subcontext-oldest-ctime-display,schema:recursive-subcontext-oldest-ctime-month-granularity,schema:recursive-subcontext-oldest-ctime-url,schema:recursive-subcontext-oldest-ctime-year-granularity,schema:recursive-subcontext-oldest-mtime,schema:recursive-subcontext-oldest-mtime-ctime,schema:recursive-subcontext-oldest-mtime-day-granularity,schema:recursive-subcontext-oldest-mtime-display,schema:recursive-subcontext-oldest-mtime-month-granularity,schema:recursive-subcontext-oldest-mtime-url,schema:recursive-subcontext-oldest-mtime-year-granularity,schema:recursive-subcontext-size,schema:recursive-subcontext-size-cdrom-count,schema:recursive-subcontext-size-dvd-count,schema:recursive-subcontext-size-human-readable,schema:recursive-subcontext-size-in-blocks,schema:recursive-subcontext-size-in-blocks-cdrom-count,schema:recursive-subcontext-size-in-blocks-dvd-count,schema:recursive-subcontext-size-in-blocks-human-readable,schema:rgba-32bpp,schema:rpm-buildtime,schema:rpm-buildtime-ctime,schema:rpm-buildtime-day-granularity,schema:rpm-buildtime-display,schema:rpm-buildtime-month-granularity,schema:rpm-buildtime-year-granularity,schema:rpm-distribution,schema:rpm-group,schema:rpm-info-url,schema:rpm-is-config,schema:rpm-is-doc,schema:rpm-is-ghost,schema:rpm-is-license,schema:rpm-is-pubkey,schema:rpm-is-readme,schema:rpm-license,schema:rpm-package,schema:rpm-packager,schema:rpm-release,schema:rpm-summary,schema:rpm-vendor,schema:rpm-verify-device,schema:rpm-verify-group,schema:rpm-verify-md5,schema:rpm-verify-mode,schema:rpm-verify-mtime,schema:rpm-verify-owner,schema:rpm-verify-size,schema:rpm-version,schema:runable,schema:selinux-context,schema:selinux-identity,schema:selinux-type,schema:sha1,schema:size,schema:size-cdrom-count,schema:size-dvd-count,schema:size-human-readable,schema:subcontext-count,schema:subcontext-dir-count,schema:subcontext-file-count,schema:subcontext-hardlink-count,schema:subcontext-oldest-atime,schema:subcontext-oldest-atime-ctime,schema:subcontext-oldest-atime-day-granularity,schema:subcontext-oldest-atime-display,schema:subcontext-oldest-atime-month-granularity,schema:subcontext-oldest-atime-url,schema:subcontext-oldest-atime-year-granularity,schema:subcontext-oldest-ctime,schema:subcontext-oldest-ctime-ctime,schema:subcontext-oldest-ctime-day-granularity,schema:subcontext-oldest-ctime-display,schema:subcontext-oldest-ctime-month-granularity,schema:subcontext-oldest-ctime-url,schema:subcontext-oldest-ctime-year-granularity,schema:subcontext-oldest-mtime,schema:subcontext-oldest-mtime-ctime,schema:subcontext-oldest-mtime-day-granularity,schema:subcontext-oldest-mtime-display,schema:subcontext-oldest-mtime-month-granularity,schema:subcontext-oldest-mtime-url,schema:subcontext-oldest-mtime-year-granularity,schema:subcontext-size,schema:subcontext-size-cdrom-count,schema:subcontext-size-dvd-count,schema:subcontext-size-human-readable,schema:subcontext-size-in-blocks,schema:subcontext-size-in-blocks-cdrom-count,schema:subcontext-size-in-blocks-dvd-count,schema:subcontext-size-in-blocks-human-readable,schema:treeicon,schema:url,schema:user-executable,schema:user-owner-name,schema:user-owner-number,schema:user-readable,schema:user-writable,schema:width,schema:writable,selinux-context,selinux-identity,selinux-type,sha1,size,size-cdrom-count,size-dvd-count,size-human-readable,subcontext-count,subcontext-dir-count,subcontext-file-count,subcontext-hardlink-count,subcontext-oldest-atime,subcontext-oldest-atime-ctime,subcontext-oldest-atime-day-granularity,subcontext-oldest-atime-display,subcontext-oldest-atime-month-granularity,subcontext-oldest-atime-url,subcontext-oldest-atime-year-granularity,subcontext-oldest-ctime,subcontext-oldest-ctime-ctime,subcontext-oldest-ctime-day-granularity,subcontext-oldest-ctime-display,subcontext-oldest-ctime-month-granularity,subcontext-oldest-ctime-url,subcontext-oldest-ctime-year-granularity,subcontext-oldest-mtime,subcontext-oldest-mtime-ctime,subcontext-oldest-mtime-day-granularity,subcontext-oldest-mtime-display,subcontext-oldest-mtime-month-granularity,subcontext-oldest-mtime-url,subcontext-oldest-mtime-year-granularity,subcontext-size,subcontext-size-cdrom-count,subcontext-size-dvd-count,subcontext-size-human-readable,subcontext-size-in-blocks,subcontext-size-in-blocks-cdrom-count,subcontext-size-in-blocks-dvd-count,subcontext-size-in-blocks-human-readable,treeicon,url,user-executable,user-owner-name,user-owner-number,user-readable,user-writable,width,writable,schema:xfs-ea-names,xfs-ea-names</keyval>\n"
"\n"
"  <keyval key=\"ea-names-no-qual\">as-rdf,as-text,as-xml,aspect-ratio,associated-branches,associated-branches-url,atime,atime-ctime,atime-day-granularity,atime-display,atime-month-granularity,atime-year-granularity,attribute-count,block-count,block-size,branchfs-attributes,branchfs-extents,branchfs-medallions,branchfs-parents,branchfs-signatures,content,ctime,ctime-ctime,ctime-day-granularity,ctime-display,ctime-month-granularity,ctime-year-granularity,deletable,depth,depth-per-color,device,device-type,dontfollow-block-count,dontfollow-block-size,dontfollow-ctime,dontfollow-device,dontfollow-device-type,dontfollow-filesystem-filetype,dontfollow-group-owner-name,dontfollow-group-owner-number,dontfollow-hard-link-count,dontfollow-has-holes,dontfollow-inode,dontfollow-is-dir,dontfollow-is-file,dontfollow-is-link,dontfollow-is-special,dontfollow-selinux-context,dontfollow-selinux-type,dontfollow-size,dontfollow-user-owner-name,dontfollow-user-owner-number,download-if-mtime-since,download-if-mtime-since-ctime,download-if-mtime-since-day-granularity,download-if-mtime-since-display,download-if-mtime-since-month-granularity,download-if-mtime-since-year-granularity,ea-names,ea-names-no-qual,emblem:generic-user-mtime,emblem:generic-user-mtime-ctime,emblem:generic-user-mtime-day-granularity,emblem:generic-user-mtime-display,emblem:generic-user-mtime-month-granularity,emblem:generic-user-mtime-year-granularity,emblem:has-fuzzy-generic-user,emblem:has-fuzzy-libferris,emblem:has-fuzzy-personalities,emblem:has-fuzzy-system,emblem:has-fuzzy-user,emblem:has-generic-user,emblem:has-libferris,emblem:has-personalities,emblem:has-system,emblem:has-user,emblem:libferris-mtime,emblem:libferris-mtime-ctime,emblem:libferris-mtime-day-granularity,emblem:libferris-mtime-display,emblem:libferris-mtime-month-granularity,emblem:libferris-mtime-year-granularity,emblem:list,emblem:list-ui,emblem:personalities-mtime,emblem:personalities-mtime-ctime,emblem:personalities-mtime-day-granularity,emblem:personalities-mtime-display,emblem:personalities-mtime-month-granularity,emblem:personalities-mtime-year-granularity,emblem:system-mtime,emblem:system-mtime-ctime,emblem:system-mtime-day-granularity,emblem:system-mtime-display,emblem:system-mtime-month-granularity,emblem:system-mtime-year-granularity,emblem:upset,emblem:user-mtime,emblem:user-mtime-ctime,emblem:user-mtime-day-granularity,emblem:user-mtime-display,emblem:user-mtime-month-granularity,emblem:user-mtime-year-granularity,exists-subdir,ferris-current-time,filesystem-filetype,force-passive-view,fs-available-block-count,fs-block-count,fs-block-size,fs-file-name-length-maximum,fs-file-nodes-free,fs-file-nodes-total,fs-free-block-count,fs-free-size,fs-id,fs-name,fs-type,gamma,google-maps-location,group-executable,group-owner-name,group-owner-number,group-readable,group-writable,hard-link-count,has-alpha,has-holes,has-subcontexts-guess,has-valid-signature,height,inode,is-active-view,is-animation-object,is-audio-object,is-dir,is-dir-try-automounting,is-file,is-image-object,is-link,is-native,is-remote,is-source-object,is-special,language-human,latitude,longitude,md2,md5,mdc2,mimetype,mimetype-from-content,mode,mtime,mtime-ctime,mtime-day-granularity,mtime-display,mtime-month-granularity,mtime-year-granularity,name,name-extension,other-executable,other-readable,other-writable,parent-name,path,preallocation-at-tail,protection-ls,protection-raw,readable,realpath,recommended-ea,recommended-ea-short,recommended-ea-union,recommended-ea-union-view,recursive-subcontext-count,recursive-subcontext-dir-count,recursive-subcontext-file-count,recursive-subcontext-hardlink-count,recursive-subcontext-max-depth,recursive-subcontext-oldest-atime,recursive-subcontext-oldest-atime-ctime,recursive-subcontext-oldest-atime-day-granularity,recursive-subcontext-oldest-atime-display,recursive-subcontext-oldest-atime-month-granularity,recursive-subcontext-oldest-atime-url,recursive-subcontext-oldest-atime-year-granularity,recursive-subcontext-oldest-ctime,recursive-subcontext-oldest-ctime-ctime,recursive-subcontext-oldest-ctime-day-granularity,recursive-subcontext-oldest-ctime-display,recursive-subcontext-oldest-ctime-month-granularity,recursive-subcontext-oldest-ctime-url,recursive-subcontext-oldest-ctime-year-granularity,recursive-subcontext-oldest-mtime,recursive-subcontext-oldest-mtime-ctime,recursive-subcontext-oldest-mtime-day-granularity,recursive-subcontext-oldest-mtime-display,recursive-subcontext-oldest-mtime-month-granularity,recursive-subcontext-oldest-mtime-url,recursive-subcontext-oldest-mtime-year-granularity,recursive-subcontext-size,recursive-subcontext-size-cdrom-count,recursive-subcontext-size-dvd-count,recursive-subcontext-size-human-readable,recursive-subcontext-size-in-blocks,recursive-subcontext-size-in-blocks-cdrom-count,recursive-subcontext-size-in-blocks-dvd-count,recursive-subcontext-size-in-blocks-human-readable,rgba-32bpp,rpm-buildtime,rpm-buildtime-ctime,rpm-buildtime-day-granularity,rpm-buildtime-display,rpm-buildtime-month-granularity,rpm-buildtime-year-granularity,rpm-distribution,rpm-group,rpm-info-url,rpm-is-config,rpm-is-doc,rpm-is-ghost,rpm-is-license,rpm-is-pubkey,rpm-is-readme,rpm-license,rpm-package,rpm-packager,rpm-release,rpm-summary,rpm-vendor,rpm-verify-device,rpm-verify-group,rpm-verify-md5,rpm-verify-mode,rpm-verify-mtime,rpm-verify-owner,rpm-verify-size,rpm-version,runable,selinux-context,selinux-identity,selinux-type,sha1,size,size-cdrom-count,size-dvd-count,size-human-readable,subcontext-count,subcontext-dir-count,subcontext-file-count,subcontext-hardlink-count,subcontext-oldest-atime,subcontext-oldest-atime-ctime,subcontext-oldest-atime-day-granularity,subcontext-oldest-atime-display,subcontext-oldest-atime-month-granularity,subcontext-oldest-atime-url,subcontext-oldest-atime-year-granularity,subcontext-oldest-ctime,subcontext-oldest-ctime-ctime,subcontext-oldest-ctime-day-granularity,subcontext-oldest-ctime-display,subcontext-oldest-ctime-month-granularity,subcontext-oldest-ctime-url,subcontext-oldest-ctime-year-granularity,subcontext-oldest-mtime,subcontext-oldest-mtime-ctime,subcontext-oldest-mtime-day-granularity,subcontext-oldest-mtime-display,subcontext-oldest-mtime-month-granularity,subcontext-oldest-mtime-url,subcontext-oldest-mtime-year-granularity,subcontext-size,subcontext-size-cdrom-count,subcontext-size-dvd-count,subcontext-size-human-readable,subcontext-size-in-blocks,subcontext-size-in-blocks-cdrom-count,subcontext-size-in-blocks-dvd-count,subcontext-size-in-blocks-human-readable,treeicon,url,user-executable,user-owner-name,user-owner-number,user-readable,user-writable,width,writable,xfs-ea-names</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:generic-user-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-fuzzy-generic-user\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-fuzzy-libferris\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-fuzzy-personalities\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-fuzzy-system\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-fuzzy-user\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-generic-user\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-libferris\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-personalities\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-system\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:has-user\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:libferris-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:list\"></keyval>\n"
"\n"
"  <keyval key=\"emblem:list-ui\"></keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:personalities-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:system-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:upset\"></keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"emblem:user-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"exists-subdir\">0</keyval>\n"
"\n"
"  <keyval key=\"ferris-current-time\">1126859498</keyval>\n"
"\n"
"  <keyval key=\"filesystem-filetype\">directory</keyval>\n"
"\n"
"  <keyval key=\"force-passive-view\">0</keyval>\n"
"\n"
"  <keyval key=\"fs-available-block-count\">1674618</keyval>\n"
"\n"
"  <keyval key=\"fs-block-count\">2283216</keyval>\n"
"\n"
"  <keyval key=\"fs-block-size\">4096</keyval>\n"
"\n"
"  <keyval key=\"fs-file-name-length-maximum\">255</keyval>\n"
"\n"
"  <keyval key=\"fs-file-nodes-free\">9057010</keyval>\n"
"\n"
"  <keyval key=\"fs-file-nodes-total\">9132864</keyval>\n"
"\n"
"  <keyval key=\"fs-free-block-count\">1674618</keyval>\n"
"\n"
"  <keyval key=\"fs-free-size\">0</keyval>\n"
"\n"
"  <keyval key=\"fs-id\">64781</keyval>\n"
"\n"
"  <keyval key=\"fs-name\">xfs</keyval>\n"
"\n"
"  <keyval key=\"fs-type\">1481003842</keyval>\n"
"\n"
"  <keyval key=\"gamma\"></keyval>\n"
"\n"
"  <keyval key=\"group-executable\">1</keyval>\n"
"\n"
"  <keyval key=\"group-owner-name\">ferristester</keyval>\n"
"\n"
"  <keyval key=\"group-owner-number\">25000</keyval>\n"
"\n"
"  <keyval key=\"group-readable\">1</keyval>\n"
"\n"
"  <keyval key=\"group-writable\">1</keyval>\n"
"\n"
"  <keyval key=\"hard-link-count\">2</keyval>\n"
"\n"
"  <keyval key=\"has-alpha\"></keyval>\n"
"\n"
"  <keyval key=\"has-holes\">0</keyval>\n"
"\n"
"  <keyval key=\"has-subcontexts-guess\">1</keyval>\n"
"\n"
"  <keyval key=\"has-valid-signature\">0</keyval>\n"
"\n"
"  <keyval key=\"height\"></keyval>\n"
"\n"
"  <keyval key=\"inode\">8481318</keyval>\n"
"\n"
"  <keyval key=\"is-active-view\">1</keyval>\n"
"\n"
"  <keyval key=\"is-animation-object\">0</keyval>\n"
"\n"
"  <keyval key=\"is-audio-object\">0</keyval>\n"
"\n"
"  <keyval key=\"is-dir\">1</keyval>\n"
"\n"
"  <keyval key=\"is-dir-try-automounting\">1</keyval>\n"
"\n"
"  <keyval key=\"is-file\">0</keyval>\n"
"\n"
"  <keyval key=\"is-image-object\">1</keyval>\n"
"\n"
"  <keyval key=\"is-link\">0</keyval>\n"
"\n"
"  <keyval key=\"is-native\">1</keyval>\n"
"\n"
"  <keyval key=\"is-remote\">0</keyval>\n"
"\n"
"  <keyval key=\"is-source-object\">0</keyval>\n"
"\n"
"  <keyval key=\"is-special\">0</keyval>\n"
"\n"
"  <keyval key=\"language-human\">unknown</keyval>\n"
"\n"
"  <keyval key=\"md2\">Read error</keyval>\n"
"\n"
"  <keyval key=\"md5\">Read error</keyval>\n"
"\n"
"  <keyval key=\"mdc2\">N/A</keyval>\n"
"\n"
"  <keyval key=\"mimetype\">inode/directory</keyval>\n"
"\n"
"  <keyval key=\"mimetype-from-content\">application/octet-stream</keyval>\n"
"\n"
"  <keyval key=\"mode\">770</keyval>\n"
"\n"
"  <keyval key=\"mtime\">1046700564</keyval>\n"
"\n"
"  <keyval key=\"mtime-ctime\">Tue Mar  4 00:09:24 2003\n"
"</keyval>\n"
"\n"
"  <keyval key=\"mtime-day-granularity\">1046700000</keyval>\n"
"\n"
"  <keyval key=\"mtime-display\">03 Mar  4 00:09</keyval>\n"
"\n"
"  <keyval key=\"mtime-month-granularity\">1046440800</keyval>\n"
"\n"
"  <keyval key=\"mtime-year-granularity\">1041343200</keyval>\n"
"\n"
"  <keyval key=\"name\">disjoint1</keyval>\n"
"\n"
"  <keyval key=\"name-extension\"></keyval>\n"
"\n"
"  <keyval key=\"other-executable\">0</keyval>\n"
"\n"
"  <keyval key=\"other-readable\">0</keyval>\n"
"\n"
"  <keyval key=\"other-writable\">0</keyval>\n"
"\n"
"  <keyval key=\"parent-name\">testbase</keyval>\n"
"\n"
"  <keyval key=\"path\">/tmp/testbase/disjoint1</keyval>\n"
"\n"
"  <keyval key=\"preallocation-at-tail\">0</keyval>\n"
"\n"
"  <keyval key=\"protection-ls\">-rwxrwx---</keyval>\n"
"\n"
"  <keyval key=\"protection-raw\">16888</keyval>\n"
"\n"
"  <keyval key=\"readable\">1</keyval>\n"
"\n"
"  <keyval key=\"realpath\">/tmp/testbase/disjoint1</keyval>\n"
"\n"
"  <keyval key=\"recommended-ea\">size-human-readable,protection-ls,mtime-display,name</keyval>\n"
"\n"
"  <keyval key=\"recommended-ea-short\">name,size-human-readable</keyval>\n"
"\n"
"  <keyval key=\"recommended-ea-union\">size-human-readable,protection-ls,mtime-display,name</keyval>\n"
"\n"
"  <keyval key=\"recommended-ea-union-view\">mtime-display,name,protection-ls,size-human-readable,</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-count\">4</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-dir-count\">1</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-file-count\">3</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-hardlink-count\">2</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-max-depth\">0</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime\">0</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-url\"></keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-atime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime\">0</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-ctime\"></keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-url\"></keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-ctime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-url\"></keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-oldest-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size\">81</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-cdrom-count\">1.10354e-07</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-dvd-count\">1.76463e-08</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-human-readable\">81</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-in-blocks\">24</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-in-blocks-cdrom-count\">3.26974e-08</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-in-blocks-dvd-count\">5.22852e-09</keyval>\n"
"\n"
"  <keyval key=\"recursive-subcontext-size-in-blocks-human-readable\">24</keyval>\n"
"\n"
"  <keyval key=\"rpm-buildtime\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-buildtime-ctime\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-buildtime-day-granularity\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-buildtime-month-granularity\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-buildtime-year-granularity\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-distribution\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-group\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-info-url\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-config\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-doc\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-ghost\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-license\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-pubkey\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-is-readme\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-license\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-package\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-packager\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-release\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-summary\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-vendor\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-device\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-group\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-md5\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-mode\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-mtime\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-owner\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-verify-size\"></keyval>\n"
"\n"
"  <keyval key=\"rpm-version\"></keyval>\n"
"\n"
"  <keyval key=\"runable\">1</keyval>\n"
"\n"
"  <keyval key=\"schema:as-rdf\">schema://xsd/attributes/string/xmldoc\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:as-text\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:as-xml\">schema://xsd/attributes/string/xmldoc\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:aspect-ratio\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:associated-branches\">schema://xsd/attributes/stringlist/urllist\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:associated-branches-url\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:atime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:attribute-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:block-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:block-size\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:branchfs-attributes\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:branchfs-extents\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:branchfs-medallions\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:branchfs-parents\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:branchfs-signatures\">schema://xsd/attributes/string/url/implicitresolvefs\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:content\">schema://xsd/attributes/binary\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ctime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:deletable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:depth\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:depth-per-color\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:device\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:device-type\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-block-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-block-size\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-ctime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-device\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-device-type\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-filesystem-filetype\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-group-owner-name\">schema://xsd/attributes/string/groupname\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-group-owner-number\">schema://xsd/attributes/decimal/integer/long/fs/gid\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-hard-link-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-has-holes\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-inode\">schema://xsd/attributes/decimal/integer/long/fs/inode\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-is-dir\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-is-file\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-is-link\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-is-special\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-selinux-context\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-selinux-type\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-size\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-user-owner-name\">schema://xsd/attributes/string/username\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:dontfollow-user-owner-number\">schema://xsd/attributes/decimal/integer/long/fs/uid\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:download-if-mtime-since-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ea-names\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ea-names-no-qual\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:generic-user-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-fuzzy-generic-user\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-fuzzy-libferris\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-fuzzy-personalities\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-fuzzy-system\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-fuzzy-user\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-generic-user\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-libferris\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-personalities\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-system\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:has-user\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:libferris-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:list\">schema://xsd/attributes/stringlist\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:list-ui\">schema://xsd/attributes/stringlist\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:personalities-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:system-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:upset\">schema://xsd/attributes/stringlist\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:emblem:user-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:exists-subdir\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:ferris-current-time\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:filesystem-filetype\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:force-passive-view\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-available-block-count\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-block-count\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-block-size\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-file-name-length-maximum\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-file-nodes-free\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-file-nodes-total\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-free-block-count\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-free-size\">schema://xsd/attributes/decimal/integer/width32\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-id\">schema://xsd/attributes/decimal/integer/long\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-name\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:fs-type\">schema://xsd/attributes/decimal/integer/long\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:gamma\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:google-maps-location\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:group-executable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:group-owner-name\">schema://xsd/attributes/string/groupname\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:group-owner-number\">schema://xsd/attributes/decimal/integer/long/fs/gid\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:group-readable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:group-writable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:hard-link-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:has-alpha\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:has-holes\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:has-subcontexts-guess\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:has-valid-signature\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:height\">schema://xsd/attributes/decimal/integer/pixelcount/height\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:inode\">schema://xsd/attributes/decimal/integer/long/fs/inode\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-active-view\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-animation-object\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-audio-object\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-dir\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-dir-try-automounting\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-file\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-image-object\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-link\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-native\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-remote\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-source-object\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:is-special\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:language-human\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:latitude\">schema://xsd/attributes/\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:longitude\">schema://xsd/attributes/\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:md2\">schema://xsd/attributes/string/digest\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:md5\">schema://xsd/attributes/string/digest\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mdc2\">schema://xsd/attributes/string/digest\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mimetype\">schema://xsd/attributes/string/mimetype\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mimetype-from-content\">schema://xsd/attributes/string/mimetype\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mode\">schema://xsd/attributes/decimal/integer/long/fs/mode\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:name\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:name-extension\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:other-executable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:other-readable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:other-writable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:parent-name\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:path\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:preallocation-at-tail\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:protection-ls\">schema://xsd/attributes/string/modestring\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:protection-raw\">schema://xsd/attributes/decimal/integer/long/fs/mode\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:readable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:realpath\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recommended-ea\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recommended-ea-short\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recommended-ea-union\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recommended-ea-union-view\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-dir-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-file-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-hardlink-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-max-depth\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-atime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-ctime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-oldest-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-cdrom-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-dvd-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-human-readable\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-in-blocks\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-in-blocks-cdrom-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-in-blocks-dvd-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:recursive-subcontext-size-in-blocks-human-readable\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rgba-32bpp\">schema://xsd/attributes/binary/rgba32bpp\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-buildtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-distribution\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-group\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-info-url\">schema://xsd/attributes/string/url/implicitresolve\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-config\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-doc\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-ghost\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-license\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-pubkey\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-is-readme\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-license\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-package\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-packager\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-release\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-summary\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-vendor\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-device\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-group\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-md5\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-mode\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-mtime\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-owner\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-verify-size\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:rpm-version\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:runable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:selinux-context\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:selinux-identity\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:selinux-type\">schema://xsd/attributes/string\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:sha1\">schema://xsd/attributes/string/digest\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:size\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:size-cdrom-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:size-dvd-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:size-human-readable\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-dir-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-file-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-hardlink-count\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-atime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-ctime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-ctime\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-day-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-display\">schema://xsd/attributes/string/unixepoch\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-month-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-oldest-mtime-year-granularity\">schema://xsd/attributes/decimal/integer/long/fs/time\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-cdrom-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-dvd-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-human-readable\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-in-blocks\">schema://xsd/attributes/decimal/integer/long/int\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-in-blocks-cdrom-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-in-blocks-dvd-count\">schema://xsd/attributes/double\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:subcontext-size-in-blocks-human-readable\">schema://xsd/attributes/decimal/integer/width64/filesize\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:treeicon\">schema://xsd/attributes/string/url/implicitresolve\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:url\">schema://xsd/attributes/string/url\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:user-executable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:user-owner-name\">schema://xsd/attributes/string/username\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:user-owner-number\">schema://xsd/attributes/decimal/integer/long/fs/uid\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:user-readable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:user-writable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:width\">schema://xsd/attributes/decimal/integer/pixelcount/width\n"
"</keyval>\n"
"\n"
"  <keyval key=\"schema:writable\">schema://xsd/attributes/boolean\n"
"</keyval>\n"
"\n"
"  <keyval key=\"sha1\">Read error</keyval>\n"
"\n"
"  <keyval key=\"size\">51</keyval>\n"
"\n"
"  <keyval key=\"size-cdrom-count\">6.9482e-08</keyval>\n"
"\n"
"  <keyval key=\"size-dvd-count\">1.11106e-08</keyval>\n"
"\n"
"  <keyval key=\"size-human-readable\">51</keyval>\n"
"\n"
"  <keyval key=\"subcontext-count\">3</keyval>\n"
"\n"
"  <keyval key=\"subcontext-dir-count\">0</keyval>\n"
"\n"
"  <keyval key=\"subcontext-file-count\">3</keyval>\n"
"\n"
"  <keyval key=\"subcontext-hardlink-count\">1</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime\">0</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-url\"></keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-atime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime\">0</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-ctime\"></keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-url\"></keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-ctime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime\">0</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-ctime\">Thu Jan  1 10:00:00 1970\n"
"</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-day-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-display\">70 Jan  1 10:00</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-month-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-url\"></keyval>\n"
"\n"
"  <keyval key=\"subcontext-oldest-mtime-year-granularity\">-36000</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size\">30</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-cdrom-count\">4.08718e-08</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-dvd-count\">6.53565e-09</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-human-readable\">30</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-in-blocks\">24</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-in-blocks-cdrom-count\">3.26974e-08</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-in-blocks-dvd-count\">5.22852e-09</keyval>\n"
"\n"
"  <keyval key=\"subcontext-size-in-blocks-human-readable\">24</keyval>\n"
"\n"
"  <keyval key=\"treeicon\">icons://ferris-mu-dir.png</keyval>\n"
"\n"
"  <keyval key=\"url\">file:///tmp/testbase/disjoint1</keyval>\n"
"\n"
"  <keyval key=\"user-executable\">1</keyval>\n"
"\n"
"  <keyval key=\"user-owner-name\">ferristester</keyval>\n"
"\n"
"  <keyval key=\"user-owner-number\">25000</keyval>\n"
"\n"
"  <keyval key=\"user-readable\">1</keyval>\n"
"\n"
"  <keyval key=\"user-writable\">1</keyval>\n"
"\n"
"  <keyval key=\"width\"></keyval>\n"
"\n"
"  <keyval key=\"writable\">1</keyval>\n"
"\n"
"  <keyval key=\"schema:xfs-ea-names\">schema://xsd/attributes/stringlist/eanames\n"
"</keyval>\n"
"\n"
"  <keyval key=\"xfs-ea-names\"></keyval>\n"
"\n"
"</context>\n"
;


fh_domdoc
runtest_StreamToDOM()
{
    fh_stringstream testdocss;
    testdocss << testdoc;
    cerr << "testdoc:" << tostr(testdocss) << endl;
    fh_domdoc    doc  = Factory::StreamToDOM( testdocss );
    DOMElement*  root = doc->getDocumentElement();

    cerr << "root:" << tostr(root->getNodeName()) << endl;
    assertcompare("document root node is not correct.",
                  "testdoc",
                  tostr(root->getNodeName()));

    DOMNodeList* nl = root->getChildNodes();
    cerr << "root node has child count:" << nl->getLength() << endl;
    assertcompare("document doesn't contain right amount of direct root children.",
                  "3", tostr( nl->getLength() ));
    
    for( int i=0; i < nl->getLength(); ++i )
    {
        DOMNode* n = nl->item( i );
        if( n->getNodeType() == DOMNode::TEXT_NODE )
        {
            DOMText* x = (DOMText*)n;
            cerr << x->getData();
        }
        if( n->getNodeType() == DOMNode::ELEMENT_NODE )
        {
            cerr << "element:" << tostr( n->getNodeName() ) << endl;
            if( tostr( n->getNodeName() ) == "childA" )
            {
                DOMNodeList* nnl = n->getChildNodes();
                assertcompare("childA doesn't contain right amount of direct root children.",
                              "1", tostr( nnl->getLength() ));
                if( nnl->getLength() )
                {
                    DOMText* x = (DOMText*)nnl->item( 0 );
                    assertcompare("childA doesn't contain right data.",
                                  "hi there", tostr( x->getData() ) );
                }
            }
        }
    }

    return doc;
}

fh_domdoc
runtest_mountDOM()
{
    fh_domdoc doc = runtest_StreamToDOM();

    fh_context c = Factory::mountDOM( doc );
    fh_context root = *(c->begin());

    assertcompare("document root node is not correct.",
                  "testdoc", root->getDirName() );

    stringlist_t expected_children;
    expected_children.push_back("childA");
    expected_children.push_back("childB");
    expected_children.push_back("childC");
    stringlist_t::iterator si = expected_children.begin();
    
    for( Context::iterator ci = root->begin(); ci != root->end(); ++ci )
    {
        cerr << " child:" << (*ci)->getURL() << endl;
        assertcompare("document root node has incorrect child.", *si, (*ci)->getDirName() );
        ++si;
    }
    
    
    return doc;
}

void
runtest_DOMToStream()
{
    fh_domdoc    doc    = runtest_StreamToDOM();
    fh_iostream  docss  = tostream( doc, false );
    string       docstr = StreamToString( docss );
    
    assertcompare("tostream( toDOM( xml )) != xml.", testdoc, docstr );

    
    {
        fh_iostream  docss  = tostream( doc, true );
        string       purdee = StreamToString( docss );
        assertcompare("tostream( toDOM( xml ), pritty) != xml.", testdoc_pritty, purdee );
    }
}

void
runtest_SetGetAttribute()
{
    string k1 = "keyNumberOne";
    string v1 = "myFirstValue";
    string k2 = "TheSecond";
    string v2 = "howdee";
    Factory::ensureXMLPlatformInitialized();
    DOMImplementation* impl = Factory::getDefaultDOMImpl();
    DOMDocument*       doc  = impl->createDocument( 0, (XMLCh*)"root", 0 );
    DOMElement*        root = doc->getDocumentElement();

    setAttribute( root, k1, v1 );
    assertcompare( "set/get attribute roundtrip failed", v1,
                   getAttribute( root, k1 ));

    setAttribute( root, k2, v2 );
    assertcompare( "set/get attribute roundtrip failed", v2,
                   getAttribute( root, k2 ));
}

void
runtest_contextToXML()
{
    fh_context c   = Resolve( BaseDir + "/disjoint1" );
    std::string cs = XML::contextToXML( c, true );

    cout << cs << flush;
    cout << "Done." << endl;
    return;

#if 0
    string msg = "contextToXML() gave different results.";
    
    fh_stringstream expectedss;
    fh_stringstream actualss;
    // Have to skip some things like atime and fs-file-nodes-free which are bound to change.
    // so we can't use assertcompare() here.
    expectedss << disjoint1_as_xml;
    actualss << cs << flush;
//     {
//       ofstream dumpss( "/tmp/actual-out" );
//       dumpss << cs << flush;
//     }
//     {
//       ofstream dumpss( "/tmp/expected-out" );
//       dumpss << disjoint1_as_xml << flush;
//     }
    for( int i=0; expectedss.good() && actualss.good(); ++i )
    {
        string eline;
        string aline;
        getline( expectedss, eline );
        getline( actualss, aline );
        if( contains( eline, "fs-file-nodes" )
            || contains( eline, "fs-available-" )
            || contains( eline, "fs-free-block" )
            || contains( eline, "atime" )
            || contains( eline, "inode" )
            || contains( eline, "taken-from-url" )
            || contains( eline, "ferris-current" )
            || contains( eline, "ctime" ) )
            continue;
        if( eline != aline )
        {
            E() << msg << " at line:" << i << endl
                << " eline:" << eline << endl
                << " aline:" << aline << endl;
        }
    }
    
    
//     assertcompare( msg,
//                    disjoint1_as_xml, cs );
//    cout << "got:" << cs << endl;
#endif
    
}

void
runtest_ReadWriteMsg()
{
    stringmap_t inv;

    inv["Greta Garbo"] = "Camille";
    inv["Cary Grant"]  = "My Wonderful Life";
    inv["Jean Harlow"] = "Bombshell";
    inv["James Cagney"] = "Blonde Crazy";
    
    fh_stringstream ss;
    cerr << "write..." << endl;
    XML::writeMessage( ss, inv );
    cerr << "XML:" << tostr(ss) << endl;
    
//     {
//         XMLCh attr1key[100];
//         XMLCh tmp[100];
//         char  ctmp[100];

//         XMLString::transcode( "pid",   attr1key,   99 );
        
//         DOMDocument* doc  = Factory::StreamToDOM( ss );
//         DOMElement*  root = doc->getDocumentElement();

//         const XMLCh* v = root->getAttribute( attr1key );
//         XMLString::transcode( v, ctmp, 99 );
//         cerr << "decoded pid:" << ctmp << endl;

//         fh_stringstream reverse = tostream( doc );
//         cerr << "reverse:" << tostr(reverse) << endl;

//         DOMNodeList* nl = root->getChildNodes();
//         cerr << "have children count:" << nl->getLength() << endl;
        
//         for( int i=0; i < nl->getLength(); ++i )
//         {
//             DOMNode* n = nl->item( i );
//             DOMElement* child = (DOMElement*)n;

//             cerr << "node type:" << n->getNodeType() << endl;
//             if( n->getNodeType() == DOMNode::ELEMENT_NODE )
//             {
//                 XMLString::transcode( "key",   attr1key,   99 );
//                 const XMLCh* v = ((DOMElement*)n)->getAttribute( attr1key );
//                 XMLString::transcode( v, ctmp, 99 );
//                 cerr << "decoded key:" << ctmp << endl;
            
//                 string sk = getAttribute( child, "key" );
//                 string sv = getAttribute( child, "value" );
//                 cerr << "k:" << sk << " v:" << sv << endl;
//             }
            
//         }
        
//         ss.clear();
//         ss.seekg(0);
//     }
    
    stringmap_t outv;
    cerr << "read..." << endl;
    XML::readMessage( ss, outv );

    cerr << "compare..." << endl;
    if( inv != outv )
    {
        E() << "readMessage( writeMessage( x ) ) != x" << endl;
        cerr << "inv follows:" << endl;
        for( stringmap_t::iterator si = inv.begin(); si != inv.end(); ++si )
            cerr << "  k:" << si->first << " v:" << si->second << endl;
        cerr << "outv follows:" << endl;
        for( stringmap_t::iterator si = outv.begin(); si != outv.end(); ++si )
            cerr << "  k:" << si->first << " v:" << si->second << endl;
    }
}

void
runtest_raw1()
{
    XMLPlatformUtils::Initialize();

    XMLCh roote[100];
    XMLCh attr1key[100];
    XMLCh attr1value[100];
    XMLCh tmp[100];
    char  ctmp[100];
    
    XMLString::transcode( "rootentity", roote,      99 );
    XMLString::transcode( "attrname",   attr1key,   99 );
    XMLString::transcode( "attrvalue",  attr1value, 99 );
    
    DOMImplementation *impl = Factory::getDefaultDOMImpl();
    DOMDocument* doc     = impl->createDocument( 0, roote, 0 );
    DOMElement* root     = doc->getDocumentElement();

    root->setAttribute( attr1key, attr1value );
    const XMLCh* v = root->getAttribute( attr1key );
    XMLString::transcode( v, ctmp, 99 );
    cerr << "decoded:" << ctmp << endl;

    char* tc = XMLString::transcode( v );
    cerr << "tc:" << tc << endl;
    XMLString::release( &tc );
}

void
runtest_raw1_ferris()
{
    XMLPlatformUtils::Initialize();
    
    XMLCh roote[100];
    XMLCh attr1key[100];
    XMLCh attr1value[100];
    XMLCh tmp[100];
    char  ctmp[100];
    
    XMLString::transcode( "rootentity", roote,      99 );
    XMLString::transcode( "attrname",   attr1key,   99 );
    XMLString::transcode( "attrvalue",  attr1value, 99 );
    
    DOMImplementation *impl = Factory::getDefaultDOMImpl();
    DOMDocument* doc     = impl->createDocument( 0, roote, 0 );
    DOMElement* root     = doc->getDocumentElement();

    setAttribute( root, "attrname", "attrkey" );
    
    const XMLCh* v = root->getAttribute( attr1key );
    XMLString::transcode( v, ctmp, 99 );
    cerr << "decoded using raw:" << ctmp << endl;

    string fv = getAttribute( root, "attrname" );
    cerr << "decoded ferris:" << fv << endl;
    
}

#include <xercesc/util/XMLUniDefs.hpp>

void
runtest_stringXMLtoStd()
{
    const string expected = "</?>=\"end";
    Factory::ensureXMLPlatformInitialized();

    const unsigned int xmlstrlen = 100;
    XMLCh xmlstr[xmlstrlen+1];
    XMLString::transcode("</?>=\"end", xmlstr, xmlstrlen );
    
    string s1 = tostr( xmlstr );
    string s2 = XMLToString( xmlstr );

    cerr << "debug, s1:" << s1 << " s2:" << s2 << endl;

    assertcompare( "tostr() API call failed.", expected, s1 );
    assertcompare( "XMLToString() API call failed.", expected, s2 );
}


void
runtest_escapeToXMLAttr()
{
    string raw      = "<tag>";
    string expected = "&lt;tag&gt;";
    std::string actual = XML::escapeToXMLAttr( raw );
    assertcompare( "XML::escapeToXMLAttr() failed", expected, actual );
}

    
    
void
runtest_updateFromXML()
{
    string parent_path = "/tmp/";
    string name = "updateFromXML.file";
    string path = parent_path + name;
    unlink( path.c_str() );
    fh_context origc  = Resolve( BaseDir + "/disjoint1/dj1fileA" );
    try { origc->read(); }
    catch( exception& e ) {}
    
    fh_context clonec = Shell::CreateFile( Resolve(parent_path), name );

    std::string xml_data = XML::contextToXML( origc );
    cerr << "runtest_updateFromXML() xml:" << xml_data << endl;
    
    XML::updateFromXML( clonec, xml_data, false );//, new EAUpdateErrorHandler_cerr() );

    
    std::string xml_data_clonec = XML::contextToXML( clonec );

    fh_context xml_origout  = Shell::acquireContext( "/tmp/xml_orig", 0, false );
    fh_context xml_cloneout = Shell::acquireContext( "/tmp/xml_clone",0, false );
    setStrAttr( xml_origout, ".",  xml_data );
    setStrAttr( xml_cloneout, ".", xml_data_clonec );


   string msg = "XML::updateFromXML() gave unexpected results.";
    
    fh_stringstream expectedss;
    fh_stringstream actualss;
    // Have to skip some things like atime and fs-file-nodes-free which are bound to change.
    // so we can't use assertcompare() here.
    expectedss << xml_data;
    actualss   << xml_data_clonec;
    for( int i=0; expectedss.good() && actualss.good(); ++i )
    {
        string eline;
        string aline;
        getline( expectedss, eline );
        getline( actualss, aline );
//        cerr << ":" << eline << ":--Ac-->:" << aline << ":" << endl;
        
        if( contains( eline, "taken-from-url" )
            || contains( eline, "fs-" )
            || contains( eline, "ctime" )
            || contains( eline, "download-if-mtime-since" )
            || contains( eline, "inode" )
            || contains( eline, "recommended-ea-union-view" )
            || contains( eline, "\"url\"" )
            || contains( eline, "\"path\"" )
            || contains( eline, "\"realpath\"" )
            || contains( eline, "\"name\"" )
            || contains( eline, "name-extension" ) )
            continue;
        if( eline != aline )
        {
            E() << msg << " at line:" << i << endl
                << " eline:" << eline << endl
                << " aline:" << aline << endl;
        }
    }
}


void
runtest_makeDOM()
{
    fh_context      c = Resolve( BaseDir );
    fh_domdoc     doc = Factory::makeDOM( c );
    DOMElement*  root = doc->getDocumentElement();

    cerr << "root:" << tostr(root->getNodeName()) << endl;
    assertcompare("document root node is not correct.",
                  c->getDirName(),
                  tostr(root->getNodeName()));

    DOMNodeList* nl = root->getChildNodes();
    cerr << "root node has child count:" << nl->getLength() << endl;
    assertcompare("document doesn't contain right amount of direct root children.",
                  "4", tostr( nl->getLength() ));

    Context::iterator ci = c->begin();
    for( int i=0; i < nl->getLength(); ++i, ++ci )
    {
        DOMNode* n = nl->item( i );
        if( n->getNodeType() == DOMNode::TEXT_NODE )
        {
            DOMText* x = (DOMText*)n;
            cerr << x->getData();
        }
        if( n->getNodeType() == DOMNode::ELEMENT_NODE )
        {
            cerr << "element:" << tostr( n->getNodeName() ) << endl;

            DOMElement* e = (DOMElement*)n;
            string name = getAttribute( e, "name" );
            
            cerr << " name:" << getAttribute( e, "name" ) << endl;
            cerr << " path:" << getAttribute( e, "path" ) << endl;
            cerr << " url:"  << getAttribute( e, "url" )  << endl;

            assertcompare( "top level subdir has incorrect name.",
                           (*ci)->getDirName(), name );

            //
            // walk a subdir just to make sure that things are as they seem
            // for nesting.
            //
            if( name == "disjoint1-conflict1" )
            {
                cerr << "=== entering subdir:" << name << " =====" << endl;
                
                DOMNodeList* nnl = n->getChildNodes();
                Context::iterator nci = (*ci)->begin();
                
                for( int ni=0; ni < nnl->getLength() && nci != (*ci)->end(); ++ni )
                {
                    DOMNode* nn = nnl->item( ni );
                    if( nn->getNodeType() == DOMNode::ELEMENT_NODE )
                    {
                        DOMElement* e = (DOMElement*)nn;
                        string name = getAttribute( e, "name" );
                        cerr << " nested name:" << name << endl;
                        fh_context xx = *nci;
                        cerr << " context name:" << xx->getDirName() << endl;
                        
                        assertcompare( "top level subdir has incorrect name.",
                                       (*nci)->getDirName(), name );
                        ++nci;
                    }
                }

                cerr << "=== exiting subdir:" << name << " =====" << endl;
            }
        }
    }
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose              = 0;
        unsigned long StreamToDOM          = 0;
        unsigned long mountDOM             = 0;
        unsigned long DOMToStream          = 0;
        unsigned long SetGetAttribute      = 0;
        unsigned long contextToXML         = 0;
        unsigned long ReadWriteMsg         = 0;
        unsigned long Raw1                 = 0;
        unsigned long Raw1Ferris           = 0;
        unsigned long EscapeToXMLAttr      = 0;
        unsigned long StringXMLtoStd       = 0;
        unsigned long UpdateFromXML        = 0;
        unsigned long MakeDOM              = 0;
        const char*   BaseDir_CSTR         = "/tmp";

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "basedir", 0, POPT_ARG_STRING, &BaseDir_CSTR, 0,
                  "where the test files are located ", "/tmp" },
                
                { "test-streamtodom", 0, POPT_ARG_NONE, &StreamToDOM, 0,
                  "test StreamToDOM() API call", "" },

                { "test-mountdom", 0, POPT_ARG_NONE, &mountDOM, 0,
                  "test mountDOM() API call", "" },

                { "test-domtostream", 0, POPT_ARG_NONE, &DOMToStream, 0,
                  "test tostream(dom) API call", "" },

                { "test-setgetattr", 0, POPT_ARG_NONE, &SetGetAttribute, 0,
                  "test set/getAttribute( DOMElement ) API calls", "" },

                { "test-contexttoxml", 0, POPT_ARG_NONE, &contextToXML, 0,
                  "test contextToXML() API call "
                  "(requires sampledata/many2onefs.tar expanded at basedir)", "" },

                { "test-readwritemsg", 0, POPT_ARG_NONE, &ReadWriteMsg, 0,
                  "test XML::writeMessage() and XML::readMessage()", "" },

                { "test-escapetoxmlattr", 0, POPT_ARG_NONE, &EscapeToXMLAttr, 0,
                  "test XML::EscapeToXMLAttr()", "" },

                { "test-stringxmltostd", 0, POPT_ARG_NONE, &StringXMLtoStd, 0,
                  "test StringXMLtoStd()", "" },

                { "test-updatefromxml", 0, POPT_ARG_NONE, &UpdateFromXML, 0,
                  "test XML::UpdateFromXML() "
                  "(requires sampledata/many2onefs.tar expanded at basedir)", "" },

                { "test-makedom", 0, POPT_ARG_NONE, &MakeDOM, 0,
                  "test MakeDOM() "
                  "(requires sampledata/many2onefs.tar expanded at basedir)", "" },


                

                { "test-raw1", 0, POPT_ARG_NONE, &Raw1, 0,
                  "simple RAW xerces-c UTF-16 testing", "" },
                { "test-raw1-ferris", 0, POPT_ARG_NONE, &Raw1Ferris, 0,
                  "simple RAW xerces-c UTF-16 testing but using libferris API", "" },

                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]*  ...");

        /* Now do options processing */
        char c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}
        BaseDir = BaseDir_CSTR;

        if( StreamToDOM )
            runtest_StreamToDOM();
        if( mountDOM )
            runtest_mountDOM();
        if( DOMToStream )
            runtest_DOMToStream();
        if( SetGetAttribute )
            runtest_SetGetAttribute();
        if( contextToXML )
            runtest_contextToXML();
        if( ReadWriteMsg )
            runtest_ReadWriteMsg();
        if( Raw1 )
            runtest_raw1();
        if( Raw1Ferris )
            runtest_raw1_ferris();
        if( EscapeToXMLAttr )
            runtest_escapeToXMLAttr();
        if( StringXMLtoStd )
            runtest_stringXMLtoStd();
        if( UpdateFromXML )
            runtest_updateFromXML();
        if( MakeDOM )
            runtest_makeDOM();
        
    }
    catch( exception& e )
    {
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
//     catch( DOMException& e )
//     {
//         cerr << "cought error: domcode:" << e.code << " domerr: " << e.msg << endl;
//         exit(1);
//     }
    if( !errors )
        cerr << "Success" << endl;
    return exit_status;
}
